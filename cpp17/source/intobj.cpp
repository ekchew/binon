#include "binon/intobj.hpp"

namespace binon {
	
	IntRangeError::IntRangeError():
		std::range_error{"C++ BinON does not support integers > 64 bits"}
	{
	}
	
	void IntObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		if(-0x40 <= mValue && mValue < 0x40) {
			WriteWord(
				static_cast<TStreamByte>(mValue & 0x7f), stream, false
				);
		}
		else if(-0x2000 <= mValue && mValue < 0x2000) {
			WriteWord(
				static_cast<std::int16_t>(0x8000 | (mValue & 0x3fff)),
				stream, false
				);
		}
		else if(-0x10000000 <= mValue && mValue < 0x10000000) {
			WriteWord(
				static_cast<std::int32_t>(0xC0000000 | (mValue & 0x1fffffff)),
				stream, false
				);
		}
		else if(-0x08000000'00000000 <= mValue && mValue < 0x08000000'00000000)
		{
			WriteWord(
				0xE0000000'00000000 | (mValue & 0x0fffffff'ffffffff),
				stream, false
				);
		}
		else {
			WriteWord('\xf0', stream, false);
			WriteWord(mValue, stream, false);
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
		auto byte0 = ReadWord<TStreamByte>(stream, false);
		if((byte0 & 0x80) == 0x00) {
			mValue = signExtend(byte0, 0x40);
		}
		else {
			std::array<TStreamByte,8> buffer;
			buffer[0] = byte0;
			if((byte0 & 0x40) == 0) {
				stream.read(buffer.data() + 1, 1);
				mValue = signExtend(
					ReadWord<std::int16_t>(buffer.data()), 0x2000
					);
			}
			else if((byte0 & 0x20) == 0) {
				stream.read(buffer.data() + 1, 3);
				mValue = signExtend(
					ReadWord<std::int32_t>(buffer.data()), 0x10000000
					);
			}
			else if((byte0 & 0x10) == 0) {
				stream.read(buffer.data() + 1, 7);
				mValue = signExtend(
					ReadWord<std::int64_t>(buffer.data()),
					0x08000000'00000000
					);
			}
			else if((byte0 & 0x01) == 0) {
				mValue = ReadWord<std::int64_t>(stream, false);
			}
			else {
				throw IntRangeError{};
			}
		}
	}
	
	void UInt::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		if(mValue < 0x80) {
			WriteWord(
				static_cast<TStreamByte>(mValue), stream, false
				);
		}
		else if(mValue < 0x4000) {
			WriteWord(
				static_cast<std::uint16_t>(0x8000 | mValue),
				stream, false
				);
		}
		else if(mValue < 0x20000000) {
			WriteWord(
				static_cast<std::uint32_t>(0xC0000000 | mValue),
				stream, false
				);
		}
		else if(mValue < 0x10000000'00000000) {
			WriteWord(
				0xE0000000'00000000 | mValue,
				stream, false
				);
		}
		else {
			WriteWord('\xf0', stream, false);
			WriteWord(mValue, stream, false);
		}
	}
	void UInt::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto byte0 = ReadWord<TStreamByte>(stream, false);
		if((byte0 & 0x80) == 0x00) {
			mValue = byte0;
		}
		else {
			std::array<TStreamByte,8> buffer;
			buffer[0] = byte0;
			if((byte0 & 0x40) == 0) {
				stream.read(buffer.data() + 1, 1);
				mValue = ReadWord<std::uint16_t>(buffer.data())
					& 0x3fffu;
			}
			else if((byte0 & 0x20) == 0) {
				stream.read(buffer.data() + 1, 3);
				mValue = ReadWord<std::uint32_t>(buffer.data())
					& 0x1fffffffu;
			}
			else if((byte0 & 0x10) == 0) {
				stream.read(buffer.data() + 1, 7);
				mValue = ReadWord<std::uint64_t>(buffer.data())
					& 0x0fffffff'ffffffffu;
			}
			else if((byte0 & 0x01) == 0) {
				mValue = ReadWord<std::uint64_t>(stream, false);
			}
			else {
				throw IntRangeError{};
			}
		}
	}

}
