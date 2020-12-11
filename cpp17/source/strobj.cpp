#include "binon/strobj.hpp"
#include "binon/intobj.hpp"

namespace binon {

	void StrObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		UInt size{mValue.size()};
		size.encodeData(stream, false);
		stream.write(mValue.data(), mValue.size());
	}
	void StrObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt len;
		len.decodeData(stream, false);
		mValue.resize(len);
		stream.read(mValue.data(), mValue.size());
	}

}
