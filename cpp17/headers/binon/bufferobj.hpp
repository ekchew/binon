#ifndef BINON_BUFFEROBJ_HPP
#define BINON_BUFFEROBJ_HPP

#include "byteutil.hpp"
#include "hystr.hpp"
#include "intobj.hpp"
#include <ostream>

namespace binon {

	struct BufferVal: BasicHyStr<std::byte> {
		using BasicHyStr<std::byte>::BasicHyStr;
		BufferVal(const HyStr& hyStr);
	};
	auto operator<< (std::ostream& stream, const BufferVal& v)
		-> std::ostream&;

	struct BufferObj:
		StdAcc<BufferObj>,
		StdEq<BufferObj>,
		StdHash<BufferObj>,
		StdCodec<BufferObj>
	{
		using TValue = BufferVal;
		static constexpr auto kTypeCode = kBufferObjCode;
		static constexpr auto kClsName = std::string_view{"BufferObj"};
		TValue mValue;
		BufferObj(const HyStr& hyStr);
		BufferObj(TValue v);
		BufferObj() = default;
		auto hasDefVal() const noexcept -> bool;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const BufferObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> BufferObj&;
		void printArgs(std::ostream& stream) const;
	};
}

namespace std {
	template<> struct hash<binon::BufferVal> {
		auto operator () (const binon::BufferVal& obj) const noexcept
			-> std::size_t;
	};
}

#endif
