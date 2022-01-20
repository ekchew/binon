#include "binon/nullobj.hpp"
#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"
#include "binon/floatobj.hpp"
#include "binon/bufferobj.hpp"
#include "binon/strobj.hpp"
#include "binon/listobj.hpp"
#include "binon/dictobj.hpp"

#include <sstream>

namespace binon {

	//---- BinOnObj ------------------------------------------------------------

	auto BinONObj::Decode(TIStream& stream, bool requireIO)
		-> TSPBinONObj
	{
		RequireIO rio{stream, requireIO};
		CodeByte cb = CodeByte::Read(stream, kSkipRequireIO);
		TSPBinONObj p = FromCodeByte(cb);
		if(Subtype(cb) != Subtype::kDefault) {
			p->decodeData(stream, kSkipRequireIO);
		}
		return p;
	}
	auto BinONObj::FromCodeByte(CodeByte cb) -> TSPBinONObj {
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
			p = std::make_shared<UIntObj>();
			break;
		case kFloatObjCode.toInt<int>():
			p = std::make_shared<FloatObj>();
			break;
		case kFloat32Code.toInt<int>():
			p = std::make_shared<Float32Obj>();
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
	void BinONObj::encode(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		CodeByte cb = typeCode();
		bool hasDefV = !*this;
		if(hasDefV) {
			Subtype{cb} = 0;
		}
		cb.write(stream, kSkipRequireIO);
		if(!hasDefV) {
			encodeData(stream, kSkipRequireIO);
		}
	}
	void BinONObj::decode(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		CodeByte cb = CodeByte::Read(stream, kSkipRequireIO);
		if(cb.typeCode() != typeCode()) {
			std::ostringstream oss;
			oss << "expected ";
			typeCode().printRepr(oss);
			oss << " but read ";
			cb.printRepr(oss);
			throw TypeErr{std::move(oss).str()};
		}
		if(Subtype(cb) != Subtype::kDefault) {
			decodeData(stream, kSkipRequireIO);
		}
	}
	void BinONObj::printRepr(std::ostream& stream) const {
		stream << clsName() << '{';
		printArgsRepr(stream);
		stream << '}';
	}
	void BinONObj::printPtrRepr(std::ostream& stream) const {
		stream << "make_shared<" << clsName() << ">(";
		if(*this) {
			printArgsRepr(stream);
		}
		stream << ')';
	}

	auto operator == (const BinONObj& lhs, const BinONObj& rhs) -> bool
		{ return lhs.equals(rhs); }
	auto operator != (const BinONObj& lhs, const BinONObj& rhs) -> bool
		{ return !lhs.equals(rhs); }
	auto operator == (const TSPBinONObj& pLHS, const TSPBinONObj& pRHS) -> bool
	{
		return pLHS->equals(*pRHS);
	}
	auto operator != (const TSPBinONObj& pLHS, const TSPBinONObj& pRHS) -> bool
	{
		return !pLHS->equals(*pRHS);
	}
	auto operator << (std::ostream& stream, const BinONObj& obj)
		-> std::ostream&
	{
		return obj.printRepr(stream), stream;
	}
	auto operator << (std::ostream& stream, const TSPBinONObj& pObj)
		-> std::ostream&
	{
		return pObj->printPtrRepr(stream), stream;
	}

}
