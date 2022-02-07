#include "binon/binonobj.hpp"

#include <iostream>

namespace binon {
	auto BinONObj::Decode(TIStream& stream, bool requireIO) -> BinONObj {
		RequireIO rio{stream, requireIO};
		CodeByte cb = CodeByte::Read(stream, kSkipRequireIO);
		auto varObj{FromTypeCode(cb.typeCode())};
		std::visit(
			[&](auto& obj) { obj.decode(cb, stream, kSkipRequireIO); },
			static_cast<BinONVariant&>(varObj)
			);
		return varObj;
	}
	auto BinONObj::FromTypeCode(CodeByte typeCode) -> BinONObj {
		switch(typeCode.asUInt()) {
			case kNullObjCode.asUInt():
				return NullObj{};
			case kBoolObjCode.asUInt(): [[fallthrough]];
			case kTrueObjCode.asUInt():
				return BoolObj{};
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
	auto BinONObj::encode(TOStream& stream, bool requireIO) const
		-> const BinONObj&
	{
		std::visit(
			[&](const auto& obj) { obj.encode(stream, requireIO); },
		 	*static_cast<const BinONVariant*>(this)
			);
		return *this;
	}
	auto BinONObj::encodeData(TOStream& stream, bool requireIO) const
		-> const BinONObj&
	{
		std::visit(
			[&](const auto& obj) { obj.encodeData(stream, requireIO); },
			*static_cast<const BinONVariant*>(this)
			);
		return *this;
	}
	auto BinONObj::decodeData(TIStream& stream, bool requireIO)
		-> BinONObj&
	{
		std::visit(
			[&](auto& obj) { obj.decodeData(stream, requireIO); },
			*static_cast<BinONVariant*>(this)
			);
		return *this;
	}
	void BinONObj::print(OptRef<std::ostream> optStream) const {
		auto& stream = optStream.value_or(std::cout).get();
		std::visit(
			[&stream](const auto& obj) {
					stream << obj.kClsName << '(';
					obj.printArgs(stream);
					stream << ')';
				},
			*static_cast<const BinONVariant*>(this)
			);
	}
	auto BinONObj::typeCode() const -> CodeByte {
		return std::visit(
			[](const auto& obj) -> CodeByte { return obj.kTypeCode; },
			*static_cast<const BinONVariant*>(this)
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
		static_cast<const binon::BinONVariant&>(obj)
	) ^ salt;
}
