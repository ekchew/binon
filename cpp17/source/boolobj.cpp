#include "binon/boolobj.hpp"

namespace binon {

	void BoolObj::encodeData(OStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		WriteWord(static_cast<StreamByte>(mValue ? 1 : 0), stream, false);
	}
	void BoolObj::decodeData(IStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto c = ReadWord<StreamByte>(stream, false);
		mValue = c != 00;
	}

}
