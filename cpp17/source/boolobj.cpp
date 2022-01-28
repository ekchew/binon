#include "binon/boolobj.hpp"

namespace binon {

	//---- TBoolObj ------------------------------------------------------------

	auto TBoolObj::encode(TOStream& stream, bool requireIO) const
		-> const TBoolObj&
	{
		CodeByte cb;
		if(mValue) {
			cb = kTrueObjCode;
		}
		else {
			cb = kTypeCode;
			Subtype{cb} = Subtype::kDefault;
		}
		cb.write(stream, requireIO);
		return *this;
	}
	auto TBoolObj::decode(CodeByte cb, TIStream& stream, bool requireIO)
		-> TBoolObj&
	{
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
		return *this;
	}
	auto TBoolObj::encodeData(TOStream& stream, bool requireIO) const
		-> const TBoolObj&
	{
		std::byte byt = mValue ? 0x01_byte : 0x00_byte;
		WriteWord(byt, stream, requireIO);
		return *this;
	}
	auto TBoolObj::decodeData(TIStream& stream, bool requireIO)
		-> TBoolObj&
	{
		mValue = ReadWord<std::byte>(stream, requireIO) != 0x00_byte;
		return *this;
	}
	void TBoolObj::printArgs(std::ostream& stream) const {
		stream << (mValue ? "true" : "false");
	}
}
