#include "binon/strobj.hpp"
#include "binon/intobj.hpp"

namespace binon {

	//---- TStrObj -------------------------------------------------------------

	TStrObj::TStrObj(TValue v):
		mValue{v}
	{
	}
	auto TStrObj::hasDefVal() const noexcept -> bool {
		return mValue.size() == 0;
	}
	auto TStrObj::encodeData(TOStream& stream, bool requireIO) const
		-> const TStrObj&
	{
		RequireIO rio{stream, requireIO};
		TUIntObj{mValue.size()}.encodeData(stream, kSkipRequireIO);
		stream.write(mValue.data(), mValue.size());
		return *this;
	}
	auto TStrObj::decodeData(TIStream& stream, bool requireIO)
		-> TStrObj&
	{
		RequireIO rio{stream, requireIO};
		TUIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		mValue.resize(n);
		stream.read(mValue.data(), n);
		return *this;
	}
	void TStrObj::printArgs(std::ostream& stream) const {
		stream << '"' << mValue << '"';
	}

	//--------------------------------------------------------------------------

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
