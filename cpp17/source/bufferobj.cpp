#include "binon/bufferobj.hpp"
#include "binon/intobj.hpp"

namespace binon {

	auto BufferObj::operator==(const BufferObj& rhs) const noexcept -> bool {
		return
			mValue.size() == rhs.mValue.size() &&
			std::equal(BINON_PAR_UNSEQ
				mValue.begin(), mValue.end(), rhs.mValue.begin()
				);
	}
	void BufferObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		UInt size{mValue.size()};
		size.encodeData(stream, kSkipRequireIO);
		stream.write(
			reinterpret_cast<const TStreamByte*>(mValue.data()),
			mValue.size()
			);
	}
	void BufferObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt len;
		len.decodeData(stream, kSkipRequireIO);
		mValue.resize(len);
		stream.read(
			reinterpret_cast<TStreamByte*>(mValue.data()),
			mValue.size()
		);
	}
	auto BufferObj::hash() const noexcept -> std::size_t {
		TStringView sv{
			reinterpret_cast<const TStreamByte*>(mValue.data()),
			mValue.size()
		};
		return std::hash<TStringView>{}(sv);
	}

}
