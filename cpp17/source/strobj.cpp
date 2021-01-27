#include "binon/strobj.hpp"
#include "binon/intobj.hpp"

namespace binon {
	void StrObjVal::makeString() {
		if(!isString()) {
			auto& sv = std::get<1>(mVariant);
			mVariant = std::string(sv.begin(), sv.end());
		}
	}

	void StrObj::EncodeData(
		const TValue& v, TOStream& stream, bool requireIO)
	{
		RequireIO rio{stream, requireIO};
		UIntObj::EncodeData(v.size(), stream, kSkipRequireIO);
		stream.write(v.data(), v.size());
	}
	auto StrObj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		RequireIO rio{stream, requireIO};
		auto size = UIntObj::DecodeData(stream, kSkipRequireIO);
		TValue v;
		v.resize(size);
		stream.read(v.data(), size);
		return std::move(v);
	}
	void StrObj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void StrObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}

}
