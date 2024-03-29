#include "binon/boolobj.hpp"

namespace binon {

	//---- BoolObj ------------------------------------------------------------

	auto BoolObj::encode(TOStream& stream, bool requireIO) const
		-> const BoolObj&
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
	auto BoolObj::decode(CodeByte cb, TIStream& stream, bool requireIO)
		-> BoolObj&
	{
		switch(cb.asUInt()) {
			case kBoolObjCode.asUInt():
				mValue = ByteUnpack<std::byte>(stream, requireIO) != 0x00_byte;
				break;
			case kTrueObjCode.asUInt():
				mValue = true;
				break;
			default: // assume default BoolObj (code 0x10)
				mValue = false;
		}
		return *this;
	}
	auto BoolObj::encodeData(TOStream& stream, bool requireIO) const
		-> const BoolObj&
	{
		std::byte byt = mValue ? 0x01_byte : 0x00_byte;
		BytePack(byt, stream, requireIO);
		return *this;
	}
	auto BoolObj::decodeData(TIStream& stream, bool requireIO)
		-> BoolObj&
	{
		mValue = ByteUnpack<std::byte>(stream, requireIO) != 0x00_byte;
		return *this;
	}
	void BoolObj::printArgs(std::ostream& stream) const {
		stream << (mValue ? "true" : "false");
	}
}
