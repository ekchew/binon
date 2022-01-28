#ifndef BINON_BUFFEROBJ_HPP
#define BINON_BUFFEROBJ_HPP

#include "byteutil.hpp"
#include "hystr.hpp"
#include "intobj.hpp"
#include <ostream>

namespace binon {

	struct TBufferVal: BasicHyStr<std::byte> {
		using BasicHyStr<std::byte>::BasicHyStr;
		TBufferVal(const HyStr& hyStr);
	};
	auto operator<< (std::ostream& stream, const TBufferVal& v)
		-> std::ostream&;

	struct TBufferObj:
		TStdAcc<TBufferObj>,
		TStdEq<TBufferObj>,
		TStdHash<TBufferObj>,
		TStdCodec<TBufferObj>
	{
		using TValue = TBufferVal;
		static constexpr auto kTypeCode = kBufferObjCode;
		static constexpr auto kClsName = std::string_view{"TBufferObj"};
		TValue mValue;
		TBufferObj(const HyStr& hyStr);
		TBufferObj(TValue v);
		TBufferObj() = default;
		auto hasDefVal() const noexcept -> bool;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TBufferObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TBufferObj&;
		void printArgs(std::ostream& stream) const;
	};
}

namespace std {
	template<> struct hash<binon::TBufferVal> {
		auto operator () (const binon::TBufferVal& obj) const noexcept
			-> std::size_t;
	};
}

#endif
