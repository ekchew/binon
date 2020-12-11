#include "binon/boolobj.hpp"

namespace binon {

	void BoolObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		WriteWord(static_cast<TStreamByte>(mValue ? 1 : 0), stream, false);
	}
	void BoolObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto c = ReadWord<TStreamByte>(stream, false);
		mValue = c != 00;
	}

}
