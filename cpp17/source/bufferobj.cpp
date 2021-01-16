#include "binon/bufferobj.hpp"
#include "binon/intobj.hpp"

namespace binon {

	void BufferObj::EncodeData(
		const TValue& v, TOStream& stream, bool requireIO)
	{
		RequireIO rio{stream, requireIO};
		UIntObj::EncodeData(v.size(), stream, kSkipRequireIO);
		auto data = reinterpret_cast<const TStreamByte*>(v.data());
		stream.write(data, v.size());
	}
	auto BufferObj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		RequireIO rio{stream, requireIO};
		auto size = UIntObj::DecodeData(stream, kSkipRequireIO);
		TValue v(size);
		auto data = reinterpret_cast<TStreamByte*>(v.data());
		stream.read(data, size);
		return std::move(v);
	}
	auto BufferObj::operator==(const BufferObj& rhs) const noexcept -> bool {
		return
			mValue.size() == rhs.mValue.size() &&
			std::memcmp(mValue.data(), rhs.mValue.data(), mValue.size()) == 0;
	}
	void BufferObj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void BufferObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
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
