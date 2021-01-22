#include "binon/bufferobj.hpp"
#include "binon/intobj.hpp"

#include <cstring>

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
	auto BufferObj::equals(const BinONObj& other) const -> bool {
		if(other.typeCode() != kBufferObjCode) {
			return false;
		}
		auto& buf0 = mValue;
		auto& buf1 = static_cast<const BufferObj&>(other).mValue;
		return buf0.size() == buf1.size()
			&& std::memcmp(buf0.data(), buf1.data(), buf0.size()) == 0;
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
