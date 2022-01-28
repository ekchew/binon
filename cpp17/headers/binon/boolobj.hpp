#ifndef BINON_BOOLOBJ_HPP
#define BINON_BOOLOBJ_HPP

#include "mixins.hpp"

namespace binon {
	struct BoolObj:
		StdAcc<BoolObj>,
		StdEq<BoolObj>,
		StdHash<BoolObj>,
		StdHasDefVal<BoolObj>
	{
		using TValue = bool;
		TValue mValue;
		static constexpr auto kTypeCode = kBoolObjCode;
		static constexpr auto kClsName = std::string_view{"BoolObj"};
		constexpr BoolObj(TValue v = false) noexcept: mValue{v} {}
		auto encode(TOStream& stream, bool requireIO = true) const
			-> const BoolObj&;
		auto decode(CodeByte cb, TIStream& stream, bool requireIO = true)
			-> BoolObj&;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const BoolObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> BoolObj&;
		void printArgs(std::ostream& stream) const;
	};
}

#endif
