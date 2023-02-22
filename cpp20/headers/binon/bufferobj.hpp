#ifndef BINON_BUFFEROBJ_HPP
#define BINON_BUFFEROBJ_HPP

#include "intobj.hpp"
#include <ostream>

namespace binon {

	//	The value of a BufferObj is implemented as hybrid string (see hystr.hpp)
	//	similar to StrObj except that each "character" is a std::byte rather
	//	than a char.
	//
	//	Printing a BufferVal will display the string as escaped hexadecimal
	//	codes along the lines "\xc0\xff\xee".
	struct BufferVal: BasicHyStr<std::byte> {
		using BasicHyStr<std::byte>::BasicHyStr;
		BufferVal(const HyStr& hyStr);
	};
	auto operator<< (std::ostream& stream, const BufferVal& v)
		-> std::ostream&;

	//	Note: The common interface to all BinONObj types is described in
	//	mixins.hpp.
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
		auto operator== (const BufferObj& rhs) const noexcept
			{ return equals(rhs); }
		auto operator!= (const BufferObj& rhs) const noexcept
			{ return !equals(rhs); }
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
