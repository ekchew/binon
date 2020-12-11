#include "binon/listobj.hpp"
#include "binon/intobj.hpp"

#include <algorithm>

namespace binon {

	ListObj::ListObj(const TValue& v):
		BinONObj{v.size() == 0}, mValue(v.size())
	{
		std::transform(
			v.begin(), v.end(), mValue.begin(),
			[](const TValue::value_type& p) { return p->makeCopy(); }
			);
	}
	void ListObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		UInt size{mValue.size()};
		size.encodeData(stream, false);
		for(auto&& pObj: mValue) {
			pObj->encode(stream, false);
		}
	}
	void ListObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt len;
		len.decodeData(stream, false);
		mValue.resize(len);
		for(auto&& pObj: mValue) {
			pObj = Decode(stream, false);
		}
	}

}
