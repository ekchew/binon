#include "binon/boolobj.hpp"

namespace binon {

	void BoolObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		WriteWord(static_cast<std::byte>(mValue ? 1 : 0), stream, false);
	}
	void BoolObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto c = ReadWord<std::byte>(stream, false);
		mValue = c != 00_byte;
	}

}
