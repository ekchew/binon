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
	static auto BytesFromHex(const HyStr& hex, std::size_t nDigits) {
		std::stringstream ss;
		auto gen = HexCodeGen(hex, ss);
		std::vector<std::byte> v;
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
		return v;
	}

	//---- IntVal --------------------------------------------------------------

	auto IntVal::FromHex(const HyStr& hex) -> IntVal {
		auto n = CountHexDigits(hex);
		auto bytes{BytesFromHex(hex, n)};
		if((n & 0x1) != 0x0 && (bytes[0] & 0x08_byte) != 0x00_byte) {
			bytes[0] |= 0xf0_byte;
		}
		if(n > sizeof(TFixed) << 1) {
			return bytes;
		}
		return IntVal{bytes}.asFixed();
	}
	auto IntVal::asHex(
		bool zerox, std::size_t wordSize
		) const -> std::string
	{
		using std::get_if;
		using std::to_integer;
		using std::setfill;
		using std::setw;
		std::ostringstream oss;
		oss << std::hex;
		if(zerox) {
			oss << "0x";
		}
		auto n = byteCount();
		auto m = n % wordSize;
		if(m > 0u) {
			m = wordSize - m;
		}
		if(isFixed()) {
			auto i = *get_if<TFixed>(this);
			if(m > 0u) {
				const char* s = i < 0 ? "ff" : "00";
				while(m-->0) {
					oss << s;
				}
			}
			oss << setfill('0');
			while(n-->0) {
				oss << setw(2) << ((i >> (n << 3)) & 0xff);
			}
		}
		else {
			auto& v = *get_if<TVariable>(this);
			if(m > 0u) {
				bool neg = v.size() > 0 && (v[0] & 0x80_byte) != 0x00_byte;
				oss << setfill(neg ? 'f' : '0');
				while(m-->0) {
					oss << setw(2) << 0;
				}
			}
			oss << setfill('0');
			for(auto b: v) {
				oss << setw(2) << to_integer<unsigned>(b);
			}
		}
		return oss.str();
	}
	auto IntVal::asFixed() const noexcept -> TFixed {
		using std::get_if;
		if(isFixed()) {
			return *get_if<TFixed>(this);
		}
		TFixed i = 0;
		bool first = true;
		int j = 0;
		for(auto byt: *get_if<TVariable>(this)) {
			if(first) {
				if((byt & 0x80_byte) != 0x00_byte) {
					i = -1;
				}
				first = false;
			}
			i <<= 8;
			i |= std::to_integer<TFixed>(byt) & 0xff;
		}
		return i;
	}
	auto IntVal::byteCount() const noexcept -> std::size_t {
		using std::get_if;
		using std::size_t;
		if(isFixed()) {
			auto i = *get_if<TFixed>(this);
			std::size_t n = 0;
			int shift = 7;
			if(i < 0) {
				while(i != -1) {
					std::cerr << i << '\n';
					i >>= shift;
					shift = 8;
					++n;
				}
			}
			else {
				while(i != 0) {
					i >>= shift;
					shift = 8;
					++n;
				}
			}
			return std::min<size_t>(std::max<size_t>(n, 1), 8);
		}
		return get_if<TVariable>(this)->size();
	}

	//---- UIntVal -------------------------------------------------------------

	auto UIntVal::FromHex(const HyStr& hex) -> UIntVal {
		auto n = CountHexDigits(hex);
		UIntVal i{BytesFromHex(hex, n)};
		if(n <= sizeof(TFixed) << 1) {
			i = i.asFixed();
		}
		return i;
	}
	auto UIntVal::asHex(
		bool zerox, std::size_t wordSize
		) const -> std::string
	{
		using std::get_if;
		using std::to_integer;
		using std::setw;
		std::ostringstream oss;
		oss << std::hex << std::setfill('0');
		if(zerox) {
			oss << "0x";
		}
		auto n = byteCount();
		auto m = n % wordSize;
		if(m > 0u) {
			m = wordSize - m;
			while(m-->0) {
				oss << "00";
			}
		}
		if(isFixed()) {
			auto i = *get_if<TFixed>(this);
			while(n-->0) {
				oss << setw(2) << ((i >> (n << 3)) & 0xff);
			}
		}
		else {
			auto& v = *get_if<TVariable>(this);
			for(auto b: v) {
				oss << setw(2) << to_integer<unsigned>(b);
			}
		}
		return oss.str();
	}
	auto UIntVal::asFixed() const noexcept -> TFixed {
		using std::get_if;
		if(isFixed()) {
			return *get_if<TFixed>(this);
		}
		TFixed i = 0;
		for(auto byt: *get_if<TVariable>(this)) {
			i <<= 8;
			i |= std::to_integer<TFixed>(byt) & 0xff;
		}
		return i;
	}
	auto UIntVal::byteCount() const noexcept -> std::size_t {
		using std::get_if;
		if(isFixed()) {
			auto i = *get_if<TFixed>(this);
			std::size_t n = 0;
			while(i != 0) {
				i >>= 8;
				++n;
			}
			return std::max<std::size_t>(n, 1);
		}
		return get_if<TVariable>(this)->size();
	}

	//---- TIntObj -------------------------------------------------------------

	void TIntObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto v = mValue;
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
	void TIntObj::decodeData(TIStream& stream, bool requireIO) {
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
			else if((byte0 & 0x01_byte) == 0x00_byte) {
				v = ReadWord<std::int64_t>(stream, kSkipRequireIO);
			}
			else {
				throw IntRangeError{};
			}
		}
		mValue = v;
	}

	//---- TUIntObj ------------------------------------------------------------

	void TUIntObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto v = mValue;
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
	void TUIntObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		TValue v;
		auto byte0 = ReadWord<std::byte>(stream, kSkipRequireIO);
		if((byte0 & 0x80_byte) == 0x00_byte) {
			v = ReadWord<std::uint8_t>(&byte0);
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
			else if((byte0 & 0x01_byte) == 0x00_byte) {
				v = ReadWord<std::uint64_t>(stream, kSkipRequireIO);
			}
			else {
				throw IntRangeError{};
			}
		}
		mValue = v;
	}

	//---- IntRangeError -------------------------------------------------------

	IntRangeError::IntRangeError():
		std::range_error{"C++ BinON does not support integers > 64 bits"}
	{
	}

	void IntObj::EncodeData(TValue v, TOStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
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
	auto IntObj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
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
			else if((byte0 & 0x01_byte) == 0x00_byte) {
				v = ReadWord<std::int64_t>(stream, kSkipRequireIO);
			}
			else {
				throw IntRangeError{};
			}
		}
		return v;
	}
	void IntObj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void IntObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}

	void UIntObj::EncodeData(
		TValue v, TOStream& stream, bool requireIO)
	{
		RequireIO rio{stream, requireIO};
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
	auto UIntObj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		RequireIO rio{stream, requireIO};
		TValue v;
		auto byte0 = ReadWord<std::byte>(stream, kSkipRequireIO);
		if((byte0 & 0x80_byte) == 0x00_byte) {
			v = ReadWord<std::uint8_t>(&byte0);
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
			else if((byte0 & 0x01_byte) == 0x00_byte) {
				v = ReadWord<std::uint64_t>(stream, kSkipRequireIO);
			}
			else {
				throw IntRangeError{};
			}
		}
		return v;
	}
	void UIntObj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void UIntObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}

}
