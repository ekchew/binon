#include "binon/ioutil.hpp"

namespace binon {

	RequireIO::RequireIO(std::ios& stream, bool enable): mRefCnt{0u} {
		if(enable) {
			mPStream = &stream;
			mEx0 = stream.exceptions();
			stream.exceptions(
				std::ios::badbit | std::ios::failbit | std::ios::eofbit
			);
		}
		else {
			mPStream = nullptr;
		}
	}
	RequireIO::RequireIO(RequireIO&& rio) noexcept: mPStream{rio.mPStream} {
		rio.mPStream = nullptr;
	}
	RequireIO::~RequireIO() {
		if(mPStream) {
			mPStream->exceptions(mEx0);
		}
	}

}
