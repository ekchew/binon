#ifndef BINON_VAROBJ_HPP
#define BINON_VAROBJ_HPP

#include "nullobj.hpp"
#include "boolobj.hpp"
#include "intobj.hpp"
#include <functional>
#include <optional>
#include <type_traits>
#include <variant>

namespace binon {
	using TVarBase = std::variant<
		TNullObj,
		TBoolObj,
		TIntObj
		>;
	struct TVarObj: TVarBase
	{
		static auto Decode(TIStream& stream, bool requireIO = true) -> TVarObj;
		static auto FromCodeByte(CodeByte cb) -> TVarObj;
		using TVarBase::variant;
		auto typeCode() const -> CodeByte;
		void encode(TOStream& stream, bool requireIO = true) const;
		void print(OptRef<std::ostream> stream = std::nullopt) const;
	};
}
namespace std {
	template<> struct hash<binon::TVarObj> {
		auto operator() (const binon::TVarObj& obj) const -> std::size_t {
			return std::visit([](const auto& obj) { return obj.hash(); }, obj);
		}
	};
}

#endif
