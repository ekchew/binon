#ifndef BINON_IOUTIL_HPP
#define BINON_IOUTIL_HPP

#include "macros.hpp"

#include <cstddef>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace binon {

	//---- Binary Stream Type Definitions --------------------------------------

	/*
	Type definitions built around BINON_STREAM_BYTE

	BINON_STREAM_BYTE defaults to std::ios::char_type, which is used by the
	standard I/O and pretty much any binary streaming operations. If, for some
	reason, you decide to change this type, make sure it is still a single-byte
	type.

	Type Definitions:
		TStreamByte: BINON_STREAM_BYTE
		TStreamTraits: std::char_traits<TStreamByte>
		TIOS: std::basic_ios<TStreamByte,TStreamTraits>
		TIStream: std::basic_istream<TStreamByte,TStreamTraits>
		TOStream: std::basic_ostream<TStreamByte,TStreamTraits>
		TString:
			std::basic_string<
				TStreamByte, TStreamTraits, BINON_ALLOCATOR<TStreamByte>
			>
		TStringView = std::basic_string_view<TStreamByte,TStreamTraits>

	With default BINON_... macros, TIStream should be equivalent to
	std::istream, TString to std::string, and so on.
	*/
	using TStreamByte = BINON_STREAM_BYTE;
	using TStreamTraits = std::char_traits<TStreamByte>;
	using TIOS = std::basic_ios<TStreamByte,TStreamTraits>;
	using TIStream = std::basic_istream<TStreamByte,TStreamTraits>;
	using TOStream = std::basic_ostream<TStreamByte,TStreamTraits>;
	using TString = std::basic_string<
		TStreamByte, TStreamTraits, BINON_ALLOCATOR<TStreamByte>
		>;
	using TStringView = std::basic_string_view<TStreamByte,TStreamTraits>;
	static_assert(
		sizeof(TStreamByte) == 1,
		"BinON streams must use a 1-byte (binary) character type"
	);

	//---- Stream Exception Bit Management -------------------------------------

	/*
	RequireIO class

	To borrow a Python term, RequireIO is a context manager. When you declare a
	local variable of this type, it sets all the exception bits of a stream so
	that any errors will throw std::failure. RequireIO's destructor sets the
	bits back to whatever they had been.

	Note that you can move instances of RequireIO but NOT copy them. (The class
	would need some recursive logic to make that possible.)
	*/
	struct RequireIO {

		/*
		Constructor - stream arg variant

		There is also a move constructor (and move assignment operator), but no
		copy or default constructors.

		Args:
			stream (TIOS&): stream on which to enable exception bits
			enable (bool, optional): enable the exception bits?
				This defaults to true, but if you know the bits have already
				been set by a previous instance of RequireIO, you can pass false
				or kSkipRequireIO to effectively disable the current instance.
		*/
		RequireIO(TIOS& stream, bool enable=true);

		RequireIO(const RequireIO& rio) = delete;
		RequireIO(RequireIO&& rio) noexcept;
		auto operator = (const RequireIO& rio) noexcept -> RequireIO& = delete;
		auto operator = (RequireIO&& rio) noexcept -> RequireIO&;
		~RequireIO();

	 protected:
		TIOS* mPStream;
		TIOS::iostate mEx0;
	};
	constexpr bool kSkipRequireIO = false;
	constexpr bool kRequireIO = true;
}

#endif
