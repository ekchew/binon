#ifndef BINON_BYTEUTIL_HPP
#define BINON_BYTEUTIL_HPP

#include "ioutil.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#if __cplusplus > 201703L
	#include <bit>
#endif

namespace binon {
	
	////////////////////////////////////////////////////////////////////////////
	//
	//	std::byte Helper Functions
	//
	////////////////////////////////////////////////////////////////////////////
	
	//	You can easily convert a std::byte to an integer by calling
	//	std::to_integer<int>() on it, but there does not seem to be a
	//	corresponding to_byte(), so binon::ToByte() fills that void.
	template<typename I> constexpr auto ToByte(I i)
		{ return std::byte{static_cast<unsigned char>(i)}; }
	
	//	std::byte's implementation does not seem to include a format for byte
	//	literals. So with this user-defined literal, you can now write
	//	0x42_byte instead of std::byte{0x42}.
	inline namespace byte_literal {
		inline constexpr auto operator ""_byte(unsigned long long i)
			{ return ToByte(i); }
	}
	
	//	AsHex() converts a std::byte into a 0-padded hexadecimal C string. The
	//	captilize option would return "AB" instead of "ab". The C string is
	//	returned wrapped inside a std::array rather than a std::string to
	//	avoid dynamic allocation. You can call the array's data() method to
	//	get at the string.
	auto AsHex(std::byte value, bool capitalize=false) -> std::array<char,3>;
	
	////////////////////////////////////////////////////////////////////////////
	//
	//	Byte Order Functions
	//
	////////////////////////////////////////////////////////////////////////////
	
	//	Returns true if compile target uses a little-endian byte order or
	//	false if it is big-endian instead.
#if __cplusplus > 201703L
	constexpr auto LittleEndian() noexcept -> bool {
		return std::endian::native == std::endian::little;
	}
#else	
	auto LittleEndian() noexcept -> bool;
#endif
	
	//	The WriteWord functions write a scalar value to either a byte buffer
	//	or an output byte stream, making byte order corrections along the way
	//	to conform to BinON's big-endian format. Its counterpart, ReadWord(),
	//	returns the value you previously serialized with WriteWord().
	//	
	//	You are strongly advised to use data types that rigorously defined in
	//	terms of word size when you call these functions. For integers, you
	//	should use std::int32_t and the like (from <cstdint>). For
	//	floating-point values, you should use either TFloat32 or TFloat64 (from
	//	"binon/floattypes.hpp").
	
	//	These lower level functions work with a byte buffer you must provide.
	//	It must have a capacity of at least sizeof(Word) bytes, and ReadWord()
	//	expects the buffer to contain your value in big-endian format.
	//
	//	WARNING:
	//		ReadWord() will reverse the byte order in your buffer if the
	//		target CPU is little-endian. You should never, therefore, call
	//		ReadWord() twice on the same buffer, as you will may a different
	//		result the second time.
	template<typename Word>
		void WriteWord(Word word, StreamByte* buffer) noexcept {
			std::memcpy(buffer, &word, sizeof(Word));
		#if __cplusplus > 201703L
			if constexpr(sizeof(Word) > 1 && LittleEndian())
		#else
			if(sizeof(Word) > 1 && LittleEndian())
		#endif
			{
				std::reverse(buffer, buffer + sizeof(Word));
			}
		}
	template<typename Word>
		auto ReadWord(StreamByte* buffer) noexcept -> Word {
		#if __cplusplus > 201703L
			if constexpr(sizeof(Word) > 1 && LittleEndian())
		#else
			if(sizeof(Word) > 1 && LittleEndian())
		#endif
			{
				std::reverse(buffer, buffer + sizeof(Word));
			}
			Word word;
			std::memcpy(&word, buffer, sizeof(Word));
			return word;
		}
	
	//	These higher level versions of the ReadWord/WriteWord functions take
	//	I/O streams instead of buffers. The extra "requireIO" argument tells
	//	the functions to set the exception bits so that the I/O will throw a
	//	std::failure if anything goes wrong. Since neither function returns
	//	any sort of error code, you probably want this and it is set true by
	//	default. But perhaps you have already set the exception bits (see the
	//	RequireIO class in binon/ioutil.hpp), in which case you make it false.
	//
	//	Note that if you are reading/writing a single byte, you may want to
	//	use the StreamByte (a.k.a. char) type to invoke a template
	//	specialization that is likely faster.
	template<typename Word>
		void WriteWord(Word word, OStream& stream, bool requireIO=true) {
			RequireIO rio{stream, requireIO};
			std::array<StreamByte, sizeof(Word)> buffer;
			WriteWord(word, buffer.data());
			stream.write(buffer.data(), buffer.size());
		}
	template<> inline
		void WriteWord<StreamByte>(
			StreamByte b, OStream& stream, bool requireIO
			)
		{ stream.write(&b, 1); }
	template<typename Word>
		auto ReadWord(IStream& stream, bool requireIO=true) -> Word {
			RequireIO rio{stream, requireIO};
			std::array<StreamByte, sizeof(Word)> buffer;
			stream.read(buffer.data(), buffer.size());
			return ReadWord<Word>(buffer.data());
		}
	template<> inline
		auto ReadWord<StreamByte>(IStream& stream, bool requireIO) -> StreamByte
		{ StreamByte b; return stream.read(&b, 1), b; }
}

#endif
