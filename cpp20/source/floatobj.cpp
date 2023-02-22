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
		BytePack<8U>(stream, mValue, requireIO);
		return *this;
	}
	auto FloatObj::decodeData(TIStream& stream, bool requireIO)
		-> FloatObj&
	{
		mValue = ByteUnpack<TValue, 8U>(stream, requireIO);
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
		BytePack<4U>(stream, mValue, requireIO);
		return *this;
	}
	auto Float32Obj::decodeData(TIStream& stream, bool requireIO)
		-> Float32Obj&
	{
		mValue = ByteUnpack<TValue, 4U>(stream, requireIO);
		return *this;
	}
}
