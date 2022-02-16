#include "binon/binonobj.hpp"
#include "binon/objhelpers.hpp"

#include <iostream>

namespace binon {
	auto BinONObj::Decode(TIStream& stream, bool requireIO) -> BinONObj {
		RequireIO rio{stream, requireIO};
		CodeByte cb = CodeByte::Read(stream, kSkipRequireIO);
		auto varObj{FromTypeCode(cb.typeCode())};
		std::visit(
			[&](auto& obj) { obj.decode(cb, stream, kSkipRequireIO); },
			varObj.value()
			);
		return varObj;
	}
	auto BinONObj::FromTypeCode(CodeByte typeCode) -> BinONObj {
		switch(typeCode.asUInt()) {
			case kNullObjCode.asUInt():
				return NullObj{};
			case kBoolObjCode.asUInt():
				return BoolObj{};
			case kTrueObjCode.asUInt():
				return BoolObj{true};
			case kIntObjCode.asUInt():
				return IntObj{};
			case kUIntCode.asUInt():
				return UIntObj{};
			case kFloatObjCode.asUInt():
				return FloatObj{};
			case kFloat32Code.asUInt():
				return Float32Obj{};
			case kBufferObjCode.asUInt():
				return BufferObj{};
			case kStrObjCode.asUInt():
				return StrObj{};
			case kListObjCode.asUInt():
				return ListObj{};
			case kSListCode.asUInt():
				return SList{};
			case kDictObjCode.asUInt():
				return DictObj{};
			case kSKDictCode.asUInt():
				return SKDict{};
			case kSDictCode.asUInt():
				return SDict{};
			default:
				throw BadCodeByte{typeCode};
		}
	}
	auto BinONObj::asTypeCodeObj(CodeByte typeCode) const& -> BinONObj {
		switch(typeCode.asUInt()) {
			case kNullObjCode.asUInt():
				return GetObj<NullObj>(*this);
			case kBoolObjCode.asUInt():
				return GetObj<BoolObj>(*this);
			case kTrueObjCode.asUInt(): {
				auto boolObj{GetObj<BoolObj>(*this)};
				if(!boolObj.value()) {
					throw BadTypeConv{"BoolObj could not convert to TrueObj"};
				}
				return boolObj;
			}
			case kIntObjCode.asUInt():
				return GetObj<IntObj>(*this);
			case kUIntCode.asUInt():
				return GetObj<UIntObj>(*this);
			case kFloatObjCode.asUInt():
				return GetObj<FloatObj>(*this);
			case kFloat32Code.asUInt():
				return GetObj<Float32Obj>(*this);
			case kBufferObjCode.asUInt():
				return GetObj<BufferObj>(*this);
			case kStrObjCode.asUInt():
				return GetObj<StrObj>(*this);
			case kListObjCode.asUInt():
				return GetObj<ListObj>(*this);
			case kSListCode.asUInt():
				return GetObj<SList>(*this);
			case kDictObjCode.asUInt():
				return GetObj<DictObj>(*this);
			case kSKDictCode.asUInt():
				return GetObj<SKDict>(*this);
			case kSDictCode.asUInt():
				return GetObj<SDict>(*this);
			default:
				throw BadCodeByte{typeCode};
		}
	}
	auto BinONObj::asTypeCodeObj(CodeByte typeCode) && -> BinONObj {
		using std::move;
		switch(typeCode.asUInt()) {
			case kNullObjCode.asUInt():
				return GetObj<NullObj>(move(*this));
			case kBoolObjCode.asUInt():
				return GetObj<BoolObj>(move(*this));
			case kTrueObjCode.asUInt(): {
				auto boolObj{GetObj<BoolObj>(move(*this))};
				if(!boolObj.value()) {
					throw BadTypeConv{"BoolObj could not convert to TrueObj"};
				}
				return move(boolObj);
			}
			case kIntObjCode.asUInt():
				return GetObj<IntObj>(move(*this));
			case kUIntCode.asUInt():
				return GetObj<UIntObj>(move(*this));
			case kFloatObjCode.asUInt():
				return GetObj<FloatObj>(move(*this));
			case kFloat32Code.asUInt():
				return GetObj<Float32Obj>(move(*this));
			case kBufferObjCode.asUInt():
				return GetObj<BufferObj>(move(*this));
			case kStrObjCode.asUInt():
				return GetObj<StrObj>(move(*this));
			case kListObjCode.asUInt():
				return GetObj<ListObj>(move(*this));
			case kSListCode.asUInt():
				return GetObj<SList>(move(*this));
			case kDictObjCode.asUInt():
				return GetObj<DictObj>(move(*this));
			case kSKDictCode.asUInt():
				return GetObj<SKDict>(move(*this));
			case kSDictCode.asUInt():
				return GetObj<SDict>(move(*this));
			default:
				throw BadCodeByte{typeCode};
		}
	}
	auto BinONObj::encode(TOStream& stream, bool requireIO) const
		-> const BinONObj&
	{
		std::visit(
			[&](const auto& obj) { obj.encode(stream, requireIO); },
		 	value()
			);
		return *this;
	}
	auto BinONObj::encodeData(TOStream& stream, bool requireIO) const
		-> const BinONObj&
	{
		std::visit(
			[&](const auto& obj) { obj.encodeData(stream, requireIO); },
			value()
			);
		return *this;
	}
	auto BinONObj::decodeData(TIStream& stream, bool requireIO)
		-> BinONObj&
	{
		std::visit(
			[&](auto& obj) { obj.decodeData(stream, requireIO); },
			value()
			);
		return *this;
	}
	void BinONObj::print(OptRef<std::ostream> optStream) const {
		auto& stream = optStream.value_or(std::cout);
		std::visit(
			[&stream](const auto& obj) {
					stream << obj.kClsName << '(';
					obj.printArgs(stream);
					stream << ')';
				},
			value()
			);
	}
	auto BinONObj::typeCode() const -> CodeByte {
		return std::visit(
			[](const auto& obj) -> CodeByte { return obj.kTypeCode; },
			value()
			);
	}
	auto operator<< (
		std::ostream& stream, const BinONObj& obj
		) -> std::ostream&
	{
		obj.print(stream);
		return stream;
	}
}

auto std::hash<binon::BinONObj>::operator() (const binon::BinONObj& obj) const
	-> std::size_t
{
	using std::size_t;
	auto msb = (sizeof(size_t) << 3) - 1;
	size_t salt = binon::gHashSalt;
	salt = salt << 1 | salt >> msb & 0x1;
	return std::visit(
		[](const auto& obj) -> size_t { return obj.hash(); },
		obj.value()
	) ^ salt;
}
