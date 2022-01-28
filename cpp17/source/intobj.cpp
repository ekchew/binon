#include "binon/intobj.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace binon {

	//---- Local functions -----------------------------------------------------

	static auto FilterHex(const HyStr& hex) {
		auto iter = hex.begin();
		for(; iter != hex.end(); ++iter) {
			if(std::tolower(*iter) == 'x') {
				break;
			}
		}
		if(iter == hex.end()) {
			iter = hex.begin();
		}
		return [&hex, iter]() mutable -> char {
			for(; iter != hex.end(); ++iter) {
				if(std::isxdigit(*iter)) {
					return *iter++;
				}
			}
			return '\xff';
		};
	}
	static auto CountHexDigits(const HyStr& hex) {
		std::size_t n = 0;
		auto filter = FilterHex(hex);
		while(filter() != '\xff') {
			++n;
		}
		return n;
	}
	static auto HexCodeGen(const HyStr& hex, std::stringstream& ss) {
		ss << std::hex;
		auto filter = FilterHex(hex);
		return [&ss, filter]() mutable -> std::byte {
			auto c = filter();
			if(c == '\xff') {
				return 0xff_byte;
			}
			ss.seekp(0);
			ss.seekg(0);
			ss << c;
			int digit;
			ss >> digit;
			return ToByte(digit & 0xf);
		};
	}
	static void BytesFromHex(
		TIntVal::TVect& v, const HyStr& hex, std::size_t nDigits
		)
	{
		std::stringstream ss;
		auto gen = HexCodeGen(hex, ss);
		v.clear();
		v.reserve((nDigits + 1) >> 1);
		bool flush = (nDigits & 0x1) != 0x0;
		std::byte byt = 0x00_byte;
		auto digit = gen();
		while(digit != 0xff_byte) {
			byt <<= 4;
			byt |= digit;
			if(flush) {
				v.push_back(byt);
				byt = 0x00_byte;
			}
			flush = !flush;
			digit = gen();
		}
	}
	static auto PaddedBytes(const HyStr& hex, std::size_t wordSize) {
		auto sigHex = CountHexDigits(hex);
		auto sigByt = (sigHex + 1u) >> 1;
		auto reqByt = sigByt + wordSize - 1u;
		reqByt -= reqByt % wordSize;
		auto padByt = reqByt - sigByt;
		TIntVal::TVect u;
		u.reserve(reqByt);
		while(padByt-->0u) {
			u.push_back(0x00_byte);
		}
		BytesFromHex(u, hex, sigHex);
		return u;
	}

	template<typename Scalar>
		static auto ByteGen(const std::variant<Scalar,TIntVal::TVect>& var)
			-> std::function<std::optional<std::byte>()>
		{
			using std::byte;
			using std::get;
			using std::nullopt;
			using std::optional;
			if(std::holds_alternative<Scalar>(var)) {
				auto scalar = get<Scalar>(var);
				int n = sizeof(Scalar) - 8;
				return [scalar, n]() mutable -> optional<byte> {
					if(n <= 0) {
						return nullopt;
					}
					n -= 8;
					return ToByte(scalar >> n & 0xff);
				};
			}
			else {
				auto& vect = get<TIntVal::TVect>(var);
				auto iter = vect.begin();
				return [&vect, iter]() mutable -> optional<byte> {
					if(iter == vect.end()) {
						return nullopt;
					}
					return *iter++;
				};
			}
		}
	static auto SigByteGen(
		const std::variant<TIntVal::TScalar,TIntVal::TVect>& var
		)
		-> std::function<std::optional<std::byte>()>
	{
		enum {kInit, kPosPad, kNegPad, kData};
		auto stage = kInit;
		auto byteGen = ByteGen(var);
		return [stage, byteGen]() mutable -> std::optional<std::byte> {
			std::optional<std::byte> optByte;
			for(;;) {
				auto optByte = byteGen();
				if(!optByte.has_value()) {
					if(stage != kData) {
						optByte = stage == kNegPad ? 0xff_byte : 0x00_byte;
						stage = kData;
					}
					return optByte;
				}
				switch(stage) {
				case kInit:
					switch(std::to_integer<int>(*optByte)) {
					case 0x00:
						stage = kPosPad;
						break;
					case 0xff:
						stage = kNegPad;
						break;
					default:
						stage = kData;
						return optByte;
					}
					break;
				case kPosPad:
					if(*optByte != 0x00_byte) {
						stage = kData;
						return optByte;
					}
					break;
				case kNegPad:
					if(*optByte != 0xff_byte) {
						stage = kData;
						return optByte;
					}
					break;
				default: // kData
					return optByte;
				}
			}
		};
	}
	static auto SigByteGen(
		const std::variant<TUIntVal::TScalar,TUIntVal::TVect>& var
		)
		-> std::function<std::optional<std::byte>()>
	{
		bool gotData = false;
		auto byteGen = ByteGen(var);
		return [gotData, byteGen]() mutable -> std::optional<std::byte> {
			std::optional<std::byte> optByte;
			for(;;) {
				auto optByte = byteGen();
				if(!optByte.has_value()) {
					if(!gotData) {
						optByte = 0x00_byte;
						gotData = true;
					}
					return optByte;
				}
				if(gotData) {
					return optByte;
				}
				if(*optByte != 0x00_byte) {
					gotData = true;
					return optByte;
				}
			}
		};
	}
	static auto PaddedByteGen(
		const std::variant<TIntVal::TScalar,TIntVal::TVect>& var,
		std::size_t wordSize
		)
		-> std::function<std::optional<std::byte>()>
	{
		auto sbg = SigByteGen(var);
		auto padByte =
			(sbg().value() & 0x80_byte) != 0x00_byte ?
			0xff_byte : 0x00_byte;
		std::size_t n;
		for(n = 1; sbg().has_value(); ++n) {}
		auto nPad = n + wordSize - 1u;
		nPad -= nPad % wordSize + n;
		sbg = SigByteGen(var);
		return [padByte, nPad, sbg]() mutable -> std::optional<std::byte> {
			if(nPad > 0u) {
				--nPad;
				return padByte;
			}
			return sbg();
		};
	}
	static auto PaddedByteGen(
		const std::variant<TUIntVal::TScalar,TUIntVal::TVect>& var,
		std::size_t wordSize
		)
		-> std::function<std::optional<std::byte>()>
	{
		auto sbg = SigByteGen(var);
		std::size_t n;
		for(n = 0; sbg().has_value(); ++n) {}
		auto nPad = n + wordSize - 1u;
		nPad -= nPad % wordSize + n;
		sbg = SigByteGen(var);
		return [nPad, sbg]() mutable -> std::optional<std::byte> {
			if(nPad > 0u) {
				--nPad;
				return 0x00_byte;
			}
			return sbg();
		};
	}

	//---- TIntVal --------------------------------------------------------------

	auto TIntVal::FromHex(const HyStr& hex, std::size_t wordSize) -> TIntVal {
		auto u = PaddedBytes(hex, wordSize);
		if(u.size() <= sizeof(TScalar)) {
			return TIntVal{u}.asScalar();
		}
		return u;
	}
	auto TIntVal::asHex(
		bool zerox, std::size_t wordSize
		) const -> std::string
	{
		std::ostringstream oss;
		oss << std::hex << std::setfill('0');
		if(zerox) {
			oss << "0x";
		}
		auto pbg = PaddedByteGen(*this, wordSize);
		auto optByte = pbg();
		while(optByte.has_value()) {
			oss << std::setw(2) << std::to_integer<int>(*optByte);
			optByte = pbg();
		}
		return oss.str();
	}
	auto TIntVal::asScalar() const noexcept -> TScalar {
		TScalar i = 0;
		auto pbg = PaddedByteGen(*this, sizeof(TScalar));
		auto optByte = pbg();
		while(optByte.has_value()) {
			i <<= 8;
			i |= std::to_integer<TScalar>(*optByte) & 0xff;
		}
		return i;
	}

	//---- TUIntVal -------------------------------------------------------------

	auto TUIntVal::FromHex(const HyStr& hex, std::size_t wordSize) -> TUIntVal {
		auto u = PaddedBytes(hex, wordSize);
		if(u.size() <= sizeof(TScalar)) {
			return TUIntVal{u}.asScalar();
		}
		return u;
	}
	auto TUIntVal::asHex(
		bool zerox, std::size_t wordSize
		) const -> std::string
	{
		std::ostringstream oss;
		oss << std::hex << std::setfill('0');
		if(zerox) {
			oss << "0x";
		}
		auto pbg = PaddedByteGen(*this, wordSize);
		auto optByte = pbg();
		while(optByte.has_value()) {
			oss << std::setw(2) << std::to_integer<int>(*optByte);
			optByte = pbg();
		}
		return oss.str();
	}
	auto TUIntVal::asScalar() const noexcept -> TScalar {
		TScalar i = 0;
		auto pbg = PaddedByteGen(*this, sizeof(TScalar));
		auto optByte = pbg();
		while(optByte.has_value()) {
			i <<= 8;
			i |= std::to_integer<TScalar>(*optByte) & 0xff;
		}
		return i;
	}

	//---- TIntObj -------------------------------------------------------------

	TIntObj::TIntObj(TValue v):
		mValue{v}
	{
	}
	auto TIntObj::encodeData(TOStream& stream, bool requireIO) const
		-> const TIntObj&
	{
		RequireIO rio{stream, requireIO};
		if(mValue.canBeScalar()) {
			auto v = mValue.scalar();
			if(-0x40 <= v && v < 0x40) {
				WriteWord(ToByte(v & 0x7f), stream, kSkipRequireIO);
			}
			else if(-0x2000 <= v && v < 0x2000) {
				WriteWord(
					static_cast<std::int16_t>(0x8000 | (v & 0x3fff)),
					stream, kSkipRequireIO
					);
			}
			else if(-0x10000000 <= v && v < 0x10000000) {
				WriteWord(
					static_cast<std::int32_t>(0xC0000000 | (v & 0x1fffffff)),
					stream, kSkipRequireIO
					);
			}
			else if(-0x08000000'00000000 <= v && v < 0x08000000'00000000)
			{
				WriteWord(
					0xE0000000'00000000 | (v & 0x0fffffff'ffffffff),
					stream, kSkipRequireIO
					);
			}
			else {
				WriteWord('\xf0', stream, kSkipRequireIO);
				WriteWord(v, stream, kSkipRequireIO);
			}
		}
		else {
			WriteWord('\xf1', stream, kSkipRequireIO);
			auto& u = mValue.vect();
			for(auto b: u) {
				WriteWord(b, stream, kSkipRequireIO);
			}
		}
		return *this;
	}
	auto TIntObj::decodeData(TIStream& stream, bool requireIO)
		-> TIntObj&
	{
		auto signExtend = [](std::int64_t v, std::int64_t msbMask) {
			auto sigBits = msbMask | msbMask - 1;
			if(v & msbMask) {
				v |= ~sigBits;
			}
			else {
				v &= sigBits;
			}
			return v;
		};
		RequireIO rio{stream, requireIO};
		TValue v;
		auto byte0 = ReadWord<std::byte>(stream, kSkipRequireIO);
		if((byte0 & 0x80_byte) == 0x00_byte) {
			v = signExtend(ReadWord<std::int8_t>(&byte0), 0x40);
		}
		else if(byte0 == 0xf1_byte) {
			TUIntObj sizeObj;
			sizeObj.decodeData(stream, kSkipRequireIO);
			auto n = sizeObj.value().scalar();
			TUIntVal::TVect u;
			u.reserve(n);
			while(n-->0u) {
				u.push_back(ReadWord<std::byte>(stream));
			}
			v = std::move(u);
		}
		else {
			std::array<std::byte,8> buffer;
			auto bufPlus1 = reinterpret_cast<TStreamByte*>(buffer.data()) + 1;
			buffer[0] = byte0;
			if((byte0 & 0x40_byte) == 0x00_byte) {
				stream.read(bufPlus1, 1);
				v = signExtend(
					ReadWord<std::int16_t>(buffer.data()), 0x2000
					);
			}
			else if((byte0 & 0x20_byte) == 0x00_byte) {
				stream.read(bufPlus1, 3);
				v = signExtend(
					ReadWord<std::int32_t>(buffer.data()), 0x10000000
					);
			}
			else if((byte0 & 0x10_byte) == 0x00_byte) {
				stream.read(bufPlus1, 7);
				v = signExtend(
					ReadWord<std::int64_t>(buffer.data()),
					0x08000000'00000000
					);
			}
			else // ((byte0 & 0x01_byte) == 0x00_byte)
			{
				v = ReadWord<std::int64_t>(stream, kSkipRequireIO);
			}
		}
		mValue = v;
		return *this;
	}

	//---- TUIntObj ------------------------------------------------------------

	TUIntObj::TUIntObj(TValue v):
		mValue{v}
	{
	}
	auto TUIntObj::encodeData(TOStream& stream, bool requireIO) const
		-> const TUIntObj&
	{
		RequireIO rio{stream, requireIO};
		if(mValue.canBeScalar()) {
			auto v = mValue.scalar();
			if(v < 0x80) {
				WriteWord(ToByte(v), stream, kSkipRequireIO);
			}
			else if(v < 0x4000) {
				WriteWord(
					static_cast<std::uint16_t>(0x8000 | v),
					stream, kSkipRequireIO
					);
			}
			else if(v < 0x20000000) {
				WriteWord(
					static_cast<std::uint32_t>(0xC0000000 | v),
					stream, kSkipRequireIO
					);
			}
			else if(v < 0x10000000'00000000) {
				WriteWord(
					0xE0000000'00000000 | v,
					stream, kSkipRequireIO
					);
			}
			else {
				WriteWord('\xf0', stream, kSkipRequireIO);
				WriteWord(v, stream, kSkipRequireIO);
			}
		}
		else {
			WriteWord('\xf1', stream, kSkipRequireIO);
			auto& u = mValue.vect();
			for(auto b: u) {
				WriteWord(b, stream, kSkipRequireIO);
			}
		}
		return *this;
	}
	auto TUIntObj::decodeData(TIStream& stream, bool requireIO)
		-> TUIntObj&
	{
		RequireIO rio{stream, requireIO};
		TValue v;
		auto byte0 = ReadWord<std::byte>(stream, kSkipRequireIO);
		if((byte0 & 0x80_byte) == 0x00_byte) {
			v = ReadWord<std::uint8_t>(&byte0);
		}
		else if(byte0 == 0xf1_byte) {
			TUIntObj sizeObj;
			sizeObj.decodeData(stream, kSkipRequireIO);
			auto n = sizeObj.value().scalar();
			TUIntVal::TVect u;
			u.reserve(n);
			while(n-->0u) {
				u.push_back(ReadWord<std::byte>(stream));
			}
			v = std::move(u);
		}
		else {
			std::array<std::byte,8> buffer;
			auto bufPlus1 = reinterpret_cast<TStreamByte*>(buffer.data()) + 1;
			buffer[0] = byte0;
			if((byte0 & 0x40_byte) == 0x00_byte) {
				stream.read(bufPlus1, 1);
				v = ReadWord<std::uint16_t>(buffer.data())
					& 0x3fffu;
			}
			else if((byte0 & 0x20_byte) == 0x00_byte) {
				stream.read(bufPlus1, 3);
				v = ReadWord<std::uint32_t>(buffer.data())
					& 0x1fffffffu;
			}
			else if((byte0 & 0x10_byte) == 0x00_byte) {
				stream.read(bufPlus1, 7);
				v = ReadWord<std::uint64_t>(buffer.data())
					& 0x0fffffff'ffffffffu;
			}
			else // ((byte0 & 0x01_byte) == 0x00_byte)
			{
				v = ReadWord<std::uint64_t>(stream, kSkipRequireIO);
			}
		}
		mValue = std::move(v);
		return *this;
	}
}

auto std::hash<binon::TIntVal>::operator() (
	const binon::TIntVal& iv
	) const noexcept -> std::size_t
{
	using binon::TIntVal;
	using std::string_view;
	if(iv.canBeScalar()) {
		return std::hash<binon::TIntVal::TScalar>{}(iv.scalar());
	}
	return std::hash<binon::TIntVal::TVect>{}(iv.vect());
}
auto std::hash<binon::TUIntVal>::operator() (
	const binon::TUIntVal& iv
	) const noexcept -> std::size_t
{
	using binon::TUIntVal;
	using std::string_view;
	if(iv.canBeScalar()) {
		return std::hash<binon::TUIntVal::TScalar>{}(iv.scalar());
	}
	return std::hash<binon::TUIntVal::TVect>{}(iv.vect());
}
