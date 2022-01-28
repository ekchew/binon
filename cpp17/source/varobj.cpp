#include "binon/varobj.hpp"

#include <iostream>

namespace binon {
	auto VarObj::Decode(TIStream& stream, bool requireIO) -> VarObj {
		RequireIO rio{stream, requireIO};
		CodeByte cb = CodeByte::Read(stream, kSkipRequireIO);
		auto varObj{FromTypeCode(cb.typeCode())};
		std::visit(
			[&](auto& obj) { obj.decode(cb, stream, kSkipRequireIO); },
			varObj
			);
		return varObj;
	}
	auto VarObj::FromTypeCode(CodeByte typeCode) -> VarObj {
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
				return TListObj{};
			case kSListCode.asUInt():
				return TSList{};
			case kDictObjCode.asUInt():
				return TDictObj{};
			case kSKDictCode.asUInt():
				return TSKDict{};
			case kSDictCode.asUInt():
				return TSDict{};
			default:
				throw BadCodeByte{typeCode};
		}
	}
	auto VarObj::encode(TOStream& stream, bool requireIO) const
		-> const VarObj&
	{
		std::visit(
			[&](const auto& obj) { obj.encode(stream, requireIO); },
		 	*this
			);
		return *this;
	}
	auto VarObj::encodeData(TOStream& stream, bool requireIO) const
		-> const VarObj&
	{
		std::visit(
			[&](const auto& obj) { obj.encodeData(stream, requireIO); },
			*this
			);
		return *this;
	}
	auto VarObj::decodeData(TIStream& stream, bool requireIO)
		-> VarObj&
	{
		std::visit(
			[&](auto& obj) { obj.decodeData(stream, requireIO); },
			*this
			);
		return *this;
	}
	void VarObj::print(OptRef<std::ostream> optStream) const {
		auto& stream = optStream.value_or(std::cout).get();
		std::visit(
			[&stream](const auto& obj) {
					stream << obj.kClsName << '(';
					obj.printArgs(stream);
					stream << ')';
				},
			*this
			);
	}
	auto VarObj::typeCode() const -> CodeByte {
		return std::visit(
			[](const auto& obj) -> CodeByte { return obj.kTypeCode; },
			*this
			);
	}
	auto operator<< (
		std::ostream& stream, const VarObj& obj
		) -> std::ostream&
	{
		obj.print(stream);
		return stream;
	}
}
