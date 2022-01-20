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
				return TNullObj{};
			case kBoolObjCode.asUInt(): [[fallthrough]];
			case kTrueObjCode.asUInt():
				return TBoolObj{};
			case kIntObjCode.asUInt():
				return TIntObj{};
			case kUIntCode.asUInt():
				return TUIntObj{};
			case kListObjCode.asUInt():
				return TListObj{};
			default:
				throw BadCodeByte{typeCode};
		}
	}
	void VarObj::encode(TOStream& stream, bool requireIO) const {
		std::visit(
			[&](const auto& obj) { obj.encode(stream, requireIO); },
		 	*this
			);
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
}
