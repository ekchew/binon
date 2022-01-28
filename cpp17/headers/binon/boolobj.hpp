#ifndef BINON_BOOLOBJ_HPP
#define BINON_BOOLOBJ_HPP

#include "mixins.hpp"

namespace binon {
	struct TBoolObj:
		TStdAcc<TBoolObj>,
		TStdEq<TBoolObj>,
		TStdHash<TBoolObj>,
		TStdHasDefVal<TBoolObj>
	{
		using TValue = bool;
		TValue mValue;
		static constexpr auto kTypeCode = kBoolObjCode;
		static constexpr auto kClsName = std::string_view{"BoolObj"};
		constexpr TBoolObj(TValue v = false) noexcept: mValue{v} {}
		auto encode(TOStream& stream, bool requireIO = true) const
			-> const TBoolObj&;
		auto decode(CodeByte cb, TIStream& stream, bool requireIO = true)
			-> TBoolObj&;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TBoolObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TBoolObj&;
		void printArgs(std::ostream& stream) const;
	};
}

#endif
