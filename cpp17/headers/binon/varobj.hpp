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
		TNullObj,
		TBoolObj,
		TIntObj,
		TUIntObj,
		TFloatObj,
		TFloat32Obj,
		TBufferObj,
		TStrObj,
		TListObj,
		TSList,
		TDictObj,
		TSKDict,
		TSDict
		>;
	struct TVarObj: TVarBase
	{
		static auto Decode(TIStream& stream, bool requireIO = true) -> TVarObj;
		static auto FromTypeCode(CodeByte cb) -> TVarObj;
		using TVarBase::variant;
		auto typeCode() const -> CodeByte;
		void encode(TOStream& stream, bool requireIO = true) const;
		void print(OptRef<std::ostream> stream = std::nullopt) const;
	};
	auto operator<< (std::ostream& stream, const TVarObj& obj) -> std::ostream&;
}
namespace std {
	template<> struct hash<binon::TVarObj> {
		auto operator() (const binon::TVarObj& obj) const -> std::size_t {
			return std::visit(
				[](const auto& obj) -> std::size_t { return obj.hash(); },
				obj
				);
		}
	};
}

#endif
