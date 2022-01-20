#include "binon/boolobj.hpp"

namespace binon {

	//---- TBoolObj ------------------------------------------------------------

	void TBoolObj::encode(TOStream& stream, bool requireIO) const {
		CodeByte cb;
		if(mValue) {
			cb = kTrueObjCode;
		}
		else {
			cb = kTypeCode;
			Subtype{cb} = Subtype::kDefault;
		}
		cb.write(stream, requireIO);
	}
	void TBoolObj::decode(CodeByte cb, TIStream& stream, bool requireIO) {
		switch(cb.asUInt()) {
			case kBoolObjCode.asUInt():
				mValue = ReadWord<std::byte>(stream, requireIO) != 0x00_byte;
				break;
			case kTrueObjCode.asUInt():
				mValue = true;
				break;
			default: // assume default BoolObj (code 0x10)
				mValue = false;
		}
	}
	void TBoolObj::printArgs(std::ostream& stream) const {
		stream << (mValue ? "true" : "false");
	}
	
	//---- PackBools -----------------------------------------------------------

	void PackBools::write(bool v) {
		mByte <<= 1;
		if(v) {
			mByte |= 0x01_byte;
		}
		if((++mNBits & 0x7U) == 0x0U) {
			WriteWord(mByte, mStream, kSkipRequireIO);
			mByte = 0x00_byte;
		}
	}
	PackBools::~PackBools() {
		auto n = mNBits & 0x7U;
		if(n != 0x0U) {
			auto byt = mByte << (0x7U - n);
			WriteWord(byt, mStream, kSkipRequireIO);
		}
	}

	//---- UnpackBools ---------------------------------------------------------

	auto UnpackBools::read() -> bool {
		if((mNBits & 0x7U) == 0x0U) {
			mByte = ReadWord<decltype(mByte)>(mStream, kSkipRequireIO);
		}
		bool val = (mByte & 0x80_byte) != 0x00_byte;
		++mNBits;
		return val;
	}

	//---- BoolObj -------------------------------------------------------------

	void BoolObj::EncodeData(TValue v, TOStream& stream, bool requireIO) {
		WriteWord(v ? 0x01_byte : 0x00_byte, stream, requireIO);
	}
	auto BoolObj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		return ReadWord<std::byte>(stream, requireIO) != 0x00_byte;
	}
	void BoolObj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void BoolObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}
}
