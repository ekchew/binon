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

	//--------------------------------------------------------------------------

	void FloatObj::EncodeData(TValue data, TOStream& stream, bool requireIO) {
		WriteWord(data, stream, requireIO);
	}
	auto FloatObj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		return ReadWord<TValue>(stream, requireIO);
	}
	void FloatObj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void FloatObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}

	void Float32Obj::EncodeData(TValue data, TOStream& stream, bool requireIO) {
		WriteWord(data, stream, requireIO);
	}
	auto Float32Obj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		return ReadWord<TValue>(stream, requireIO);
	}
	void Float32Obj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void Float32Obj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}
}
