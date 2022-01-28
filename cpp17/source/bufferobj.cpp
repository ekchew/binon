#include "binon/bufferobj.hpp"
#include "binon/intobj.hpp"

#include <cstring>

namespace binon {

	//---- TBufferVal ----------------------------------------------------------

	TBufferVal::TBufferVal(const HyStr& hyStr) {
		auto n = hyStr.size();
		resize(n);
		std::memcpy(data(), hyStr.data(), n);
	}
	auto operator<< (std::ostream& stream, const TBufferVal& v) -> std::ostream&
	{
		for(auto byt: v) {
			auto hex = AsHexC(byt);
			stream << "\\x" << hex.data();
		}
		return stream;
	}

	//---- TBufferObj ----------------------------------------------------------

	TBufferObj::TBufferObj(const HyStr& hyStr):
		mValue{hyStr}
	{
	}
	TBufferObj::TBufferObj(TValue v):
		mValue{v}
	{
	}
	auto TBufferObj::hasDefVal() const noexcept -> bool {
		return mValue.size() == 0;
	}
	auto TBufferObj::encodeData(TOStream& stream, bool requireIO) const
		-> const TBufferObj&
	{
		RequireIO rio{stream, requireIO};
		TUIntObj{mValue.size()}.encodeData(stream, kSkipRequireIO);
		auto data = reinterpret_cast<const TStreamByte*>(mValue.data());
		stream.write(data, mValue.size());
		return *this;
	}
	auto TBufferObj::decodeData(TIStream& stream, bool requireIO)
		-> TBufferObj&
	{
		RequireIO rio{stream, requireIO};
		TUIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		mValue.resize(n);
		auto data = reinterpret_cast<TStreamByte*>(mValue.data());
		stream.read(data, n);
		return *this;
	}
	void TBufferObj::printArgs(std::ostream& stream) const {
		stream << '"' << mValue << '"';
	}

}
auto std::hash<binon::TBufferVal>::operator() (
	const binon::TBufferVal& obj
	) const noexcept -> std::size_t
{
	auto data = reinterpret_cast<const binon::TStreamByte*>(obj.data());
	std::string_view sv{data, obj.size()};
	return std::hash<std::string_view>{}(sv);
}
