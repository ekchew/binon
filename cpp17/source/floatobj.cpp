#include "binon/floatobj.hpp"

namespace binon {

	//---- FloatObj -----------------------------------------------------------

	FloatObj::FloatObj(TValue v):
		mValue{v}
	{
	}
	auto FloatObj::encodeData(TOStream& stream, bool requireIO) const
		-> const FloatObj&
	{
		WriteWord(mValue, stream, requireIO);
		return *this;
	}
	auto FloatObj::decodeData(TIStream& stream, bool requireIO)
		-> FloatObj&
	{
		mValue = ReadWord<TValue>(stream, requireIO);
		return *this;
	}

	//---- Float32Obj ---------------------------------------------------------

	Float32Obj::Float32Obj(TValue v):
		mValue{v}
	{
	}
	auto Float32Obj::encodeData(TOStream& stream, bool requireIO) const
		-> const Float32Obj&
	{
		WriteWord(mValue, stream, requireIO);
		return *this;
	}
	auto Float32Obj::decodeData(TIStream& stream, bool requireIO)
		-> Float32Obj&
	{
		mValue = ReadWord<TValue>(stream, requireIO);
		return *this;
	}
}
