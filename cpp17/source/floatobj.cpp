#include "binon/floatobj.hpp"

namespace binon {
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
