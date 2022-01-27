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
	void TBoolObj::encodeData(TOStream& stream, bool requireIO) const {
		std::byte byt = mValue ? 0x01_byte : 0x00_byte;
		WriteWord(byt, stream, requireIO);
	}
	void TBoolObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = ReadWord<std::byte>(stream, requireIO) != 0x00_byte;
	}
	void TBoolObj::printArgs(std::ostream& stream) const {
		stream << (mValue ? "true" : "false");
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
