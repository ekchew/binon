#include "binon/ioutil.hpp"

namespace binon {

	RequireIO::RequireIO(std::ios& stream, bool enable) {
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
	auto RequireIO::operator = (RequireIO&& rio) noexcept -> RequireIO& {
		return mPStream = rio.mPStream, rio.mPStream = nullptr, *this;
	}
	RequireIO::~RequireIO() {
		if(mPStream) {
			mPStream->exceptions(mEx0);
		}
	}

}
