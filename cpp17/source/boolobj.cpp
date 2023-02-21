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
		switch(cb.asInt()) {
			case kBoolObjCode.asInt():
				mValue = ByteUnpack<TStreamByte>(stream, requireIO) != 0;
				break;
			case kTrueObjCode.asInt():
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
		BytePack<1U>(stream, mValue ? '\1' : '\0', requireIO);
		return *this;
	}
	auto BoolObj::decodeData(TIStream& stream, bool requireIO)
		-> BoolObj&
	{
		mValue = ByteUnpack<TStreamByte>(stream, requireIO) != 0;
		return *this;
	}
	void BoolObj::printArgs(std::ostream& stream) const {
		stream << (mValue ? "true" : "false");
	}
}
