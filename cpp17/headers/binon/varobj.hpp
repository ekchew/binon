#ifndef BINON_VAROBJ_HPP
#define BINON_VAROBJ_HPP

#include "nullobj.hpp"
#include "boolobj.hpp"
#include "intobj.hpp"
#include "listobj.hpp"
#include <functional>
#include <optional>
#include <type_traits>
#include <variant>

namespace binon {
	using TVarBase = std::variant<
		TNullObj,
		TBoolObj,
		TIntObj,
		TUIntObj,
		TListObj
		>;
	struct VarObj: TVarBase
	{
		static auto Decode(TIStream& stream, bool requireIO = true) -> VarObj;
		static auto FromTypeCode(CodeByte cb) -> VarObj;
		using TVarBase::variant;
		auto typeCode() const -> CodeByte;
		void encode(TOStream& stream, bool requireIO = true) const;
		void print(OptRef<std::ostream> stream = std::nullopt) const;
	};
}
namespace std {
	template<> struct hash<binon::VarObj> {
		auto operator() (const binon::VarObj& obj) const -> std::size_t {
			return std::visit([](const auto& obj) { return obj.hash(); }, obj);
		}
	};
}

#endif
