#ifndef BINON_FLOATOBJ_HPP
#define BINON_FLOATOBJ_HPP

#include "mixins.hpp"

namespace binon {

	struct FloatObj:
		StdAcc<FloatObj>,
		StdEq<FloatObj>,
		StdHash<FloatObj>,
		StdHasDefVal<FloatObj>,
		StdPrintArgs<FloatObj>,
		StdCodec<FloatObj>
	{
		using TValue = types::TFloat64;
		static constexpr auto kTypeCode = kFloatObjCode;
		static constexpr auto kClsName = std::string_view{"FloatObj"};
		TValue mValue;
		FloatObj(TValue v = 0.0);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const FloatObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> FloatObj&;
	};
	struct Float32Obj:
		StdAcc<Float32Obj>,
		StdEq<Float32Obj>,
		StdHash<Float32Obj>,
		StdHasDefVal<Float32Obj>,
		StdPrintArgs<Float32Obj>,
		StdCodec<Float32Obj>
	{
		using TValue = types::TFloat32;
		static constexpr auto kTypeCode = kFloat32Code;
		static constexpr auto kClsName = std::string_view{"Float32Obj"};
		TValue mValue;
		Float32Obj(TValue v = 0.0f);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const Float32Obj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> Float32Obj&;
	};

	namespace types {
		using Float32 = Float32Obj;
	}
}

#endif
