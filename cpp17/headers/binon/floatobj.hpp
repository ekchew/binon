#ifndef BINON_FLOATOBJ_HPP
#define BINON_FLOATOBJ_HPP

#include "mixins.hpp"

namespace binon {

	struct TFloatObj:
		TStdAcc<TFloatObj>,
		TStdEq<TFloatObj>,
		TStdHash<TFloatObj>,
		TStdHasDefVal<TFloatObj>,
		TStdPrintArgs<TFloatObj>,
		TStdCodec<TFloatObj>
	{
		using TValue = types::TFloat64;
		static constexpr auto kTypeCode = kFloatObjCode;
		static constexpr auto kClsName = std::string_view{"TFloatObj"};
		TValue mValue;
		TFloatObj(TValue v = 0.0);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TFloatObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TFloatObj&;
	};
	struct TFloat32Obj:
		TStdAcc<TFloat32Obj>,
		TStdEq<TFloat32Obj>,
		TStdHash<TFloat32Obj>,
		TStdHasDefVal<TFloat32Obj>,
		TStdPrintArgs<TFloat32Obj>,
		TStdCodec<TFloat32Obj>
	{
		using TValue = types::TFloat32;
		static constexpr auto kTypeCode = kFloat32Code;
		static constexpr auto kClsName = std::string_view{"TFloat32Obj"};
		TValue mValue;
		TFloat32Obj(TValue v = 0.0f);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TFloat32Obj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TFloat32Obj&;
	};

	namespace types {
		using Float32 = TFloat32Obj;
	}
}

#endif
