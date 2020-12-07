#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"
#include "binon/floatobj.hpp"
#include "binon/nullobj.hpp"

namespace binon {
	
	auto BinONObj::FromNullValue() -> std::unique_ptr<BinONObj> {
		return std::make_unique<NullObj>();
	}
	auto BinONObj::FromBoolValue(bool v) -> std::unique_ptr<BinONObj> {
		std::unique_ptr<BinONObj> p;
		if(v) {
			p = std::make_unique<TrueObj>();
		}
		else {
			p = std::make_unique<BoolObj>(false);
		}
		return p;
	}
	auto BinONObj::FromCodeByte(CodeByte cb) -> std::unique_ptr<BinONObj> {
		std::unique_ptr<BinONObj> p;
		switch(cb.typeCode().toInt<int>()) {
		case kNullObjCode.toInt<int>():
			p = std::make_unique<NullObj>();
			break;
		case kBoolObjCode.toInt<int>():
			p = std::make_unique<BoolObj>();
			break;
		case kTrueObjCode.toInt<int>():
			p = std::make_unique<TrueObj>();
			break;
		case kIntObjCode.toInt<int>():
			p = std::make_unique<IntObj>();
			break;
		case kUIntCode.toInt<int>():
			p = std::make_unique<UInt>();
			break;
		case kFloatObjCode.toInt<int>():
			p = std::make_unique<FloatObj>();
			break;
		case kFloat32Code.toInt<int>():
			p = std::make_unique<Float32>();
			break;
		default:
			throw BadCodeByte{cb};
		}
		p->mHasDefVal = Subtype{cb} == Subtype::kDefault;
		return p;
	}
	auto BinONObj::Decode(IStream& stream, bool requireIO)
		-> std::unique_ptr<BinONObj>
	{
		RequireIO rio{stream, requireIO};
		auto p = FromCodeByte(CodeByte::Read(stream, false));
		if(!p->mHasDefVal) {
			p->decodeData(stream, false);
		}
		return p;
	}
	void BinONObj::encode(OStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto cb = typeCode();
		if(mHasDefVal) {
			Subtype{cb} = 0;
		}
		cb.write(stream, false);
		if(!mHasDefVal) {
			encodeData(stream, false);
		}
	}
	void BinONObj::typeErr() const {
		throw TypeErr("incorrect accessor called on BinONObj subtype");
	}
	
}
