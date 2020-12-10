#include "binon/bufferobj.hpp"
#include "binon/intobj.hpp"

namespace binon {

	void BufferObj::encodeData(OStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt size{mValue.size()};
		size.encodeData(stream, false);
		stream.write(mValue.data(), mValue.size());
	}
	void BufferObj::decodeData(IStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt len;
		len.decodeData(stream, false);
		mValue.resize(len);
		stream.read(mValue.data(), mValue.size());
	}
	auto BufferObj::hash() const noexcept -> std::size_t {
		StringView sv{mValue.data(), mValue.size()};
		return std::hash<StringView>{}(sv);
	}

}
