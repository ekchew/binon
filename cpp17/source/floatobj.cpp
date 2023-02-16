#include "binon/floatobj.hpp"

namespace binon {

	//---- FloatObj -----------------------------------------------------------

	FloatObj::FloatObj(const Float32Obj& obj):
		mValue{obj.mValue}
	{
	}
	FloatObj::FloatObj(TValue v):
		mValue{v}
	{
	}
	auto FloatObj::encodeData(TOStream& stream, bool requireIO) const
		-> const FloatObj&
	{
		BytePack(mValue, stream, requireIO);
		return *this;
	}
	auto FloatObj::decodeData(TIStream& stream, bool requireIO)
		-> FloatObj&
	{
		mValue = ByteUnpack<TValue>(stream, requireIO);
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
		BytePack(mValue, stream, requireIO);
		return *this;
	}
	auto Float32Obj::decodeData(TIStream& stream, bool requireIO)
		-> Float32Obj&
	{
		mValue = ByteUnpack<TValue>(stream, requireIO);
		return *this;
	}
}
