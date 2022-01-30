#ifndef BINON_BINONOBJ_HPP
#define BINON_BINONOBJ_HPP

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
	using BinONVariant = std::variant<
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
	struct BinONObj: BinONVariant
	{
		static auto Decode(TIStream& stream, bool requireIO = true) -> BinONObj;
		static auto FromTypeCode(CodeByte cb) -> BinONObj;
		using BinONVariant::variant;
		auto typeCode() const -> CodeByte;
		auto encode(TOStream& stream, bool requireIO = true) const
			-> const BinONObj&;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const BinONObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> BinONObj&;
		void print(OptRef<std::ostream> stream = std::nullopt) const;
	};
	auto operator<< (std::ostream& stream, const BinONObj& obj)
		-> std::ostream&;
}
namespace std {
	template<> struct hash<binon::BinONObj> {
		auto operator() (const binon::BinONObj& obj) const -> std::size_t {
			return std::visit(
				[](const auto& obj) -> std::size_t { return obj.hash(); },
				obj
				);
		}
	};
}

#endif
