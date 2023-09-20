#include "binon/bufferobj.hpp"

#include <cstring>

namespace binon {

	//---- BufferVal ----------------------------------------------------------

	BufferVal::BufferVal(const HyStr& hyStr) {
		auto n = hyStr.size();
		resize(n);
		std::memcpy(data(), hyStr.data(), n);
	}
	auto operator<< (std::ostream& stream, const BufferVal& v) -> std::ostream&
	{
		for(auto c: v) {
			stream << "\\x" << AsHex(ToByte(c));
		}
		return stream;
	}

	//---- BufferObj ----------------------------------------------------------

	BufferObj::BufferObj(const HyStr& hyStr):
		mValue{hyStr}
	{
	}
	BufferObj::BufferObj(TValue v):
		mValue{v}
	{
	}
	auto BufferObj::hasDefVal() const noexcept -> bool {
		return mValue.size() == 0;
	}
	auto BufferObj::encodeData(TOStream& stream, bool requireIO) const
		-> const BufferObj&
	{
		RequireIO rio{stream, requireIO};
		UIntObj{mValue.size()}.encodeData(stream, kSkipRequireIO);
		auto data = reinterpret_cast<const TStreamByte*>(mValue.data());
		stream.write(data, mValue.size());
		return *this;
	}
	auto BufferObj::decodeData(TIStream& stream, bool requireIO)
		-> BufferObj&
	{
		RequireIO rio{stream, requireIO};
		UIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		mValue.resize(n);
		auto data = reinterpret_cast<TStreamByte*>(mValue.data());
		stream.read(data, n);
		return *this;
	}
	void BufferObj::printArgs(std::ostream& stream) const {
		stream << '"' << mValue << '"';
	}

}
auto std::hash<binon::BufferVal>::operator() (
	const binon::BufferVal& obj
	) const noexcept -> std::size_t
{
	auto data = reinterpret_cast<const binon::TStreamByte*>(obj.data());
	std::string_view sv{data, obj.size()};
	return std::hash<std::string_view>{}(sv);
}
