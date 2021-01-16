#include "binon/boolobj.hpp"

namespace binon {

	void BoolObj::EncodeData(TValue v, TOStream& stream, bool requireIO) {
		WriteWord(v ? 0x01_byte : 0x00_byte, stream, requireIO);
	}
	auto BoolObj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		return ReadWord<std::byte>(stream, requireIO) != 0x00_byte;
	}
	void BoolObj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void BoolObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}

}
