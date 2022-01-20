#include "binon/varobj.hpp"

#include <iostream>

namespace binon {
	auto TVarObj::Decode(TIStream& stream, bool requireIO) -> TVarObj {
		RequireIO rio{stream, requireIO};
		CodeByte cb = CodeByte::Read(stream, kSkipRequireIO);
		auto varObj{FromCodeByte(cb)};
		std::visit(
			[&](auto& obj) { obj.decode(cb, stream, kSkipRequireIO); },
			varObj
			);
		return varObj;
	}
	auto TVarObj::FromCodeByte(CodeByte cb) -> TVarObj {
		switch(cb.typeCode().asUInt()) {
			case kNullObjCode.asUInt():
				return TNullObj{};
			case kBoolObjCode.asUInt(): [[fallthrough]];
			case kTrueObjCode.asUInt():
				return TBoolObj{};
			case kIntObjCode.asUInt():
				return TIntObj{};
			case kUIntCode.asUInt():
				return TUIntObj{};
			default:
				throw BadCodeByte{cb};
		}
	}
	void TVarObj::encode(TOStream& stream, bool requireIO) const {
		std::visit(
			[&](const auto& obj) { obj.encode(stream, requireIO); },
		 	*this
			);
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
}
