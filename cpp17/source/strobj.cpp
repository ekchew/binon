#include "binon/strobj.hpp"
#include "binon/intobj.hpp"

namespace binon {

	//---- StrObj -------------------------------------------------------------

	StrObj::StrObj(TValue v):
		mValue{v}
	{
	}
	auto StrObj::hasDefVal() const noexcept -> bool {
		return mValue.size() == 0;
	}
	auto StrObj::encodeData(TOStream& stream, bool requireIO) const
		-> const StrObj&
	{
		RequireIO rio{stream, requireIO};
		UIntObj{mValue.size()}.encodeData(stream, kSkipRequireIO);
		stream.write(mValue.data(), mValue.size());
		return *this;
	}
	auto StrObj::decodeData(TIStream& stream, bool requireIO)
		-> StrObj&
	{
		RequireIO rio{stream, requireIO};
		UIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		mValue.resize(n);
		stream.read(mValue.data(), n);
		return *this;
	}
	void StrObj::printArgs(std::ostream& stream) const {
		stream << '"' << mValue << '"';
	}

}
