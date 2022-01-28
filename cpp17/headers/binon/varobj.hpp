#ifndef BINON_VAROBJ_HPP
#define BINON_VAROBJ_HPP

#include "nullobj.hpp"
#include "boolobj.hpp"
#include "intobj.hpp"
#include "floatobj.hpp"
#include "bufferobj.hpp"
#include "strobj.hpp"
#include "listobj.hpp"
#include "dictobj.hpp"
#include <functional>
#include <optional>
#include <ostream>
#include <type_traits>
#include <variant>

namespace binon {
	using TVarBase = std::variant<
		NullObj,
		BoolObj,
		IntObj,
		UIntObj,
		FloatObj,
		Float32Obj,
		BufferObj,
		StrObj,
		TListObj,
		TSList,
		TDictObj,
		TSKDict,
		TSDict
		>;
	struct VarObj: TVarBase
	{
		static auto Decode(TIStream& stream, bool requireIO = true) -> VarObj;
		static auto FromTypeCode(CodeByte cb) -> VarObj;
		using TVarBase::variant;
		auto typeCode() const -> CodeByte;
		auto encode(TOStream& stream, bool requireIO = true) const
			-> const VarObj&;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const VarObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> VarObj&;
		void print(OptRef<std::ostream> stream = std::nullopt) const;
	};
	auto operator<< (std::ostream& stream, const VarObj& obj) -> std::ostream&;
}
namespace std {
	template<> struct hash<binon::VarObj> {
		auto operator() (const binon::VarObj& obj) const -> std::size_t {
			return std::visit(
				[](const auto& obj) -> std::size_t { return obj.hash(); },
				obj
				);
		}
	};
}

#endif
