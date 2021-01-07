#include "binon/strobj.hpp"
#include "binon/intobj.hpp"

namespace binon {

	void StrObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		UIntObj size{mValue.size()};
		size.encodeData(stream, kSkipRequireIO);
		stream.write(mValue.data(), mValue.size());
	}
	void StrObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UIntObj len;
		len.decodeData(stream, kSkipRequireIO);
		mValue.resize(len);
		stream.read(mValue.data(), mValue.size());
	}

}
