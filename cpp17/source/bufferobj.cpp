#include "binon/bufferobj.hpp"
#include "binon/intobj.hpp"

namespace binon {

	auto BufferObj::operator==(const BufferObj& rhs) const noexcept -> bool {
		return
			mValue.size() == rhs.mValue.size() &&
			std::memcmp(mValue.data(), rhs.mValue.data(), mValue.size()) == 0;
	}
	void BufferObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		UIntObj size{mValue.size()};
		size.encodeData(stream, kSkipRequireIO);
		stream.write(
			reinterpret_cast<const TStreamByte*>(mValue.data()),
			mValue.size()
			);
	}
	void BufferObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UIntObj len;
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
	void BufferObj::printArgsRepr(std::ostream& stream) const {
		stream << "vector{";
		bool first = true;
		for(auto byt: mValue) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			PrintByte(byt, stream);
		}
		stream << '}';
	}

}
