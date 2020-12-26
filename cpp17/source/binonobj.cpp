#include "binon/nullobj.hpp"
#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"
#include "binon/floatobj.hpp"
#include "binon/bufferobj.hpp"
#include "binon/strobj.hpp"
#include "binon/listobj.hpp"
#include "binon/dictobj.hpp"

namespace binon {
	
	auto BinONObj::FromTypeCode(CodeByte cb) -> TSPBinONObj {
		TSPBinONObj p;
		switch(cb.typeCode().toInt<int>()) {
		case kNullObjCode.toInt<int>():
			p = std::make_shared<NullObj>();
			break;
		case kBoolObjCode.toInt<int>():
			p = std::make_shared<BoolObj>();
			break;
		case kTrueObjCode.toInt<int>():
			p = std::make_shared<TrueObj>();
			break;
		case kIntObjCode.toInt<int>():
			p = std::make_shared<IntObj>();
			break;
		case kUIntCode.toInt<int>():
			p = std::make_shared<UInt>();
			break;
		case kFloatObjCode.toInt<int>():
			p = std::make_shared<FloatObj>();
			break;
		case kFloat32Code.toInt<int>():
			p = std::make_shared<Float32>();
			break;
		case kBufferObjCode.toInt<int>():
			p = std::make_shared<BufferObj>();
			break;
		case kStrObjCode.toInt<int>():
			p = std::make_shared<StrObj>();
			break;
		case kListObjCode.toInt<int>():
			p = std::make_shared<ListObj>();
			break;
		case kSListCode.toInt<int>():
			p = std::make_shared<SList>();
			break;
		case kDictObjCode.toInt<int>():
			p = std::make_shared<DictObj>();
			break;
		case kSKDictCode.toInt<int>():
			p = std::make_shared<SKDict>();
			break;
		case kSDictCode.toInt<int>():
			p = std::make_shared<SDict>();
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
	void BinONObj::printRepr(std::ostream& stream) const {
		stream << clsName() << '{';
		if(!hasDefVal()) {
			printArgsRepr(stream);
		}
		stream << '}';
	}
	void BinONObj::printPtrRepr(std:ostream& stream) const {
		stream << "make_shared<" << clsName() << ">(";
		if(!hasDefVal()) {
			printArgsRepr(stream);
		}
		stream << ')';
	}
	
	auto operator==(const TSPBinONObj& pLHS, const TSPBinONObj& pRHS) -> bool {
		return pLHS->equals(*pRHS);
	}
	
}
