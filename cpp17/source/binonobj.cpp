#include "binon/nullobj.hpp"
#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"
#include "binon/floatobj.hpp"
#include "binon/bufferobj.hpp"
#include "binon/strobj.hpp"
#include "binon/listobj.hpp"

namespace binon {
	
	auto BinONObj::FromNullValue() -> TSPBinONObj {
		return MakeSharedPtr<NullObj>();
	}
	auto BinONObj::FromBoolValue(bool v) -> TSPBinONObj {
		TSPBinONObj p;
		if(v) {
			p = MakeSharedPtr<TrueObj>();
		}
		else {
			p = MakeSharedPtr<BoolObj>(false);
		}
		return p;
	}
	auto BinONObj::FromTypeCode(CodeByte cb) -> TSPBinONObj {
		TSPBinONObj p;
		switch(cb.typeCode().toInt<int>()) {
		case kNullObjCode.toInt<int>():
			p = MakeSharedPtr<NullObj>();
			break;
		case kBoolObjCode.toInt<int>():
			p = MakeSharedPtr<BoolObj>();
			break;
		case kTrueObjCode.toInt<int>():
			p = MakeSharedPtr<TrueObj>();
			break;
		case kIntObjCode.toInt<int>():
			p = MakeSharedPtr<IntObj>();
			break;
		case kUIntCode.toInt<int>():
			p = MakeSharedPtr<UInt>();
			break;
		case kFloatObjCode.toInt<int>():
			p = MakeSharedPtr<FloatObj>();
			break;
		case kFloat32Code.toInt<int>():
			p = MakeSharedPtr<Float32>();
			break;
		case kBufferObjCode.toInt<int>():
			p = MakeSharedPtr<BufferObj>();
			break;
		case kStrObjCode.toInt<int>():
			p = MakeSharedPtr<StrObj>();
			break;
		case kListObjCode.toInt<int>():
			p = MakeSharedPtr<ListObj>();
			break;
		case kSListCode.toInt<int>():
			p = MakeSharedPtr<SList>();
			break;
		default:
			throw BadCodeByte{cb};
		}
		return p;
	}
	auto BinONObj::Decode(TIStream& stream, bool requireIO)
		-> TSPBinONObj
	{
		RequireIO rio{stream, requireIO};
		auto p = FromTypeCode(CodeByte::Read(stream, false));
		if(!p->hasDefVal()) {
			p->decodeData(stream, kSkipRequireIO);
		}
		return p;
	}
	void BinONObj::encode(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto cb = typeCode();
		bool hasDefV = hasDefVal();
		if(hasDefV) {
			Subtype{cb} = 0;
		}
		cb.write(stream, kSkipRequireIO);
		if(!hasDefV) {
			encodeData(stream, kSkipRequireIO);
		}
	}
	void BinONObj::typeErr() const {
		throw TypeErr("incorrect accessor called on BinONObj subtype");
	}
	
	auto operator==(const TSPBinONObj& pLHS, const TSPBinONObj& pRHS) -> bool {
		return pLHS->equals(*pRHS);
	}
	
}
