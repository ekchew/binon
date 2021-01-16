#include "binon/intobj.hpp"

namespace binon {

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
