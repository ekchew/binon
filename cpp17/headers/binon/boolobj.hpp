#ifndef BINON_BOOLOBJ_HPP
#define BINON_BOOLOBJ_HPP

#include "mixins.hpp"

namespace binon {
	/*
	The BoolObj struct is conceptually simple but has some quirks.
	For example, the encodeData() and decodeData() methods are normally never
	called unless you call them yourself directly. This is due to 2 reasons:

		1. When you call encode() on a scalar BoolObj, it can encode the value
		   straight into the code byte (as either a default (false) BoolObj or
		   a TrueObj, in BinON parlance).
		2. When you encode a batch of bools from say an SList, they get packed
		   8 to a byte.

	Note: The common interface to all BinONObj types is described in mixins.hpp.
	*/
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
