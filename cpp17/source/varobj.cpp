#include "binon/varobj.hpp"

#include <iostream>

namespace binon {
	auto TVarObj::Decode(TIStream& stream, bool requireIO) -> TVarObj {
		RequireIO rio{stream, requireIO};
		CodeByte cb = CodeByte::Read(stream, kSkipRequireIO);
		auto varObj{FromTypeCode(cb.typeCode())};
		std::visit(
			[&](auto& obj) { obj.decode(cb, stream, kSkipRequireIO); },
			varObj
			);
		return varObj;
	}
	auto TVarObj::FromTypeCode(CodeByte typeCode) -> TVarObj {
		switch(typeCode.asUInt()) {
			case kNullObjCode.asUInt():
				return TNullObj{};
			case kBoolObjCode.asUInt(): [[fallthrough]];
			case kTrueObjCode.asUInt():
				return TBoolObj{};
			case kIntObjCode.asUInt():
				return TIntObj{};
			case kUIntCode.asUInt():
				return TUIntObj{};
			case kFloatObjCode.asUInt():
				return TFloatObj{};
			case kFloat32Code.asUInt():
				return TFloat32Obj{};
			case kBufferObjCode.asUInt():
				return TBufferObj{};
			case kStrObjCode.asUInt():
				return TStrObj{};
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
	auto TVarObj::encode(TOStream& stream, bool requireIO) const
		-> const TVarObj&
	{
		std::visit(
			[&](const auto& obj) { obj.encode(stream, requireIO); },
		 	*this
			);
		return *this;
	}
	auto TVarObj::encodeData(TOStream& stream, bool requireIO) const
		-> const TVarObj&
	{
		std::visit(
			[&](const auto& obj) { obj.encodeData(stream, requireIO); },
			*this
			);
		return *this;
	}
	auto TVarObj::decodeData(TIStream& stream, bool requireIO)
		-> TVarObj&
	{
		std::visit(
			[&](auto& obj) { obj.decodeData(stream, requireIO); },
			*this
			);
		return *this;
	}
	void TVarObj::print(OptRef<std::ostream> optStream) const {
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
	auto TVarObj::typeCode() const -> CodeByte {
		return std::visit(
			[](const auto& obj) -> CodeByte { return obj.kTypeCode; },
			*this
			);
	}
	auto operator<< (
		std::ostream& stream, const TVarObj& obj
		) -> std::ostream&
	{
		obj.print(stream);
		return stream;
	}
}
