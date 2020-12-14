#include "binon/intobj.hpp"

namespace binon {
	
	IntRangeError::IntRangeError():
		std::range_error{"C++ BinON does not support integers > 64 bits"}
	{
	}
	
	void IntObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		if(-0x40 <= mValue && mValue < 0x40) {
			WriteWord(ToByte(mValue & 0x7f), stream, kSkipRequireIO);
		}
		else if(-0x2000 <= mValue && mValue < 0x2000) {
			WriteWord(
				static_cast<std::int16_t>(0x8000 | (mValue & 0x3fff)),
				stream, kSkipRequireIO
				);
		}
		else if(-0x10000000 <= mValue && mValue < 0x10000000) {
			WriteWord(
				static_cast<std::int32_t>(0xC0000000 | (mValue & 0x1fffffff)),
				stream, kSkipRequireIO
				);
		}
		else if(-0x08000000'00000000 <= mValue && mValue < 0x08000000'00000000)
		{
			WriteWord(
				0xE0000000'00000000 | (mValue & 0x0fffffff'ffffffff),
				stream, kSkipRequireIO
				);
		}
		else {
			WriteWord('\xf0', stream, kSkipRequireIO);
			WriteWord(mValue, stream, kSkipRequireIO);
		}
	}
	void IntObj::decodeData(TIStream& stream, bool requireIO) {
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
		auto byte0 = ReadWord<std::byte>(stream, kSkipRequireIO);
		if((byte0 & 0x80_byte) == 0x00_byte) {
			mValue = signExtend(ReadWord<std::int8_t>(&byte0), 0x40);
		}
		else {
			std::array<std::byte,8> buffer;
			auto bufPlus1 = reinterpret_cast<TStreamByte*>(buffer.data()) + 1;
			buffer[0] = byte0;
			if((byte0 & 0x40_byte) == 0x00_byte) {
				stream.read(bufPlus1, 1);
				mValue = signExtend(
					ReadWord<std::int16_t>(buffer.data()), 0x2000
					);
			}
			else if((byte0 & 0x20_byte) == 0x00_byte) {
				stream.read(bufPlus1, 3);
				mValue = signExtend(
					ReadWord<std::int32_t>(buffer.data()), 0x10000000
					);
			}
			else if((byte0 & 0x10_byte) == 0x00_byte) {
				stream.read(bufPlus1, 7);
				mValue = signExtend(
					ReadWord<std::int64_t>(buffer.data()),
					0x08000000'00000000
					);
			}
			else if((byte0 & 0x01_byte) == 0x00_byte) {
				mValue = ReadWord<std::int64_t>(stream, kSkipRequireIO);
			}
			else {
				throw IntRangeError{};
			}
		}
	}
	
	void UInt::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		if(mValue < 0x80) {
			WriteWord(ToByte(mValue), stream, kSkipRequireIO);
		}
		else if(mValue < 0x4000) {
			WriteWord(
				static_cast<std::uint16_t>(0x8000 | mValue),
				stream, kSkipRequireIO
				);
		}
		else if(mValue < 0x20000000) {
			WriteWord(
				static_cast<std::uint32_t>(0xC0000000 | mValue),
				stream, kSkipRequireIO
				);
		}
		else if(mValue < 0x10000000'00000000) {
			WriteWord(
				0xE0000000'00000000 | mValue,
				stream, kSkipRequireIO
				);
		}
		else {
			WriteWord('\xf0', stream, kSkipRequireIO);
			WriteWord(mValue, stream, kSkipRequireIO);
		}
	}
	void UInt::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto byte0 = ReadWord<std::byte>(stream, kSkipRequireIO);
		if((byte0 & 0x80_byte) == 0x00_byte) {
			mValue = ReadWord<std::uint8_t>(&byte0);
		}
		else {
			std::array<std::byte,8> buffer;
			auto bufPlus1 = reinterpret_cast<TStreamByte*>(buffer.data()) + 1;
			buffer[0] = byte0;
			if((byte0 & 0x40_byte) == 0x00_byte) {
				stream.read(bufPlus1, 1);
				mValue = ReadWord<std::uint16_t>(buffer.data())
					& 0x3fffu;
			}
			else if((byte0 & 0x20_byte) == 0x00_byte) {
				stream.read(bufPlus1, 3);
				mValue = ReadWord<std::uint32_t>(buffer.data())
					& 0x1fffffffu;
			}
			else if((byte0 & 0x10_byte) == 0x00_byte) {
				stream.read(bufPlus1, 7);
				mValue = ReadWord<std::uint64_t>(buffer.data())
					& 0x0fffffff'ffffffffu;
			}
			else if((byte0 & 0x01_byte) == 0x00_byte) {
				mValue = ReadWord<std::uint64_t>(stream, kSkipRequireIO);
			}
			else {
				throw IntRangeError{};
			}
		}
	}

}
