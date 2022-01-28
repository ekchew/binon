#include "binon/floatobj.hpp"

namespace binon {

	//---- TFloatObj -----------------------------------------------------------

	TFloatObj::TFloatObj(TValue v):
		mValue{v}
	{
	}
	auto TFloatObj::encodeData(TOStream& stream, bool requireIO) const
		-> const TFloatObj&
	{
		WriteWord(mValue, stream, requireIO);
		return *this;
	}
	auto TFloatObj::decodeData(TIStream& stream, bool requireIO)
		-> TFloatObj&
	{
		mValue = ReadWord<TValue>(stream, requireIO);
		return *this;
	}

	//---- TFloat32Obj ---------------------------------------------------------

	TFloat32Obj::TFloat32Obj(TValue v):
		mValue{v}
	{
	}
	auto TFloat32Obj::encodeData(TOStream& stream, bool requireIO) const
		-> const TFloat32Obj&
	{
		WriteWord(mValue, stream, requireIO);
		return *this;
	}
	auto TFloat32Obj::decodeData(TIStream& stream, bool requireIO)
		-> TFloat32Obj&
	{
		mValue = ReadWord<TValue>(stream, requireIO);
		return *this;
	}
}
