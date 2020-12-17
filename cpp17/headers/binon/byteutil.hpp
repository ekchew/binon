#ifndef BINON_BYTEUTIL_HPP
#define BINON_BYTEUTIL_HPP

#include "ioutil.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <string>
#if BINON_CPP20
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
		
	//	AsHexC() converts a std::byte into a 0-padded hexadecimal C string. The
	//	captilize option would return "AB" instead of "ab". The C string is
	//	returned wrapped inside a std::array rather than a std::string to
	//	avoid dynamic allocation. You can call the array's data() method to
	//	get at the string.
	auto AsHexC(std::byte value, bool capitalize=false) noexcept
		-> std::array<char,3>;
	
	//	AsHex() is the same as AsHexC() but returns a std::string instead of
	//	the lower-level char array.
	auto AsHex(std::byte value, bool capitalize=false) -> std::string;
	
	////////////////////////////////////////////////////////////////////////////
	//
	//	Byte Order Functions
	//
	////////////////////////////////////////////////////////////////////////////
	
	//	Returns true if compile target uses a little-endian byte order or
	//	false if it is big-endian instead.
#if BINON_CPP20
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
		void WriteWord(Word word, std::byte* buffer) noexcept {
			if constexpr(sizeof word == 1U) {
				buffer[0] = reinterpret_cast<std::byte&>(word);
			}
			else if constexpr(sizeof word > 1U) {
				std::memcpy(buffer, &word, sizeof(Word));
				if BINON_CPP20_CONSTEXPR(LittleEndian()) {
					std::reverse(buffer, buffer + sizeof(Word));
				}
			}
		}
	template<typename Word>
		auto ReadWord(std::byte* buffer) noexcept -> Word {
			Word word;
			if constexpr(sizeof word == 1U) {
				word = reinterpret_cast<const Word*>(buffer)[0];
			}
			else if constexpr(sizeof word > 1U) {
				if BINON_CPP20_CONSTEXPR(LittleEndian()) {
					std::reverse(buffer, buffer + sizeof word);
				}
				std::memcpy(&word, buffer, sizeof word);
			}
			return word;
		}
	
	//	These higher level versions of the ReadWord/WriteWord functions take
	//	I/O streams instead of buffers. The extra "requireIO" argument tells
	//	the functions to set the exception bits so that the I/O will throw a
	//	std::failure if anything goes wrong. Since neither function returns
	//	any sort of error code, you probably want this and it is set true by
	//	default. But perhaps you have already set the exception bits (see the
	//	RequireIO class in binon/ioutil.hpp), in which case you make it false.
	template<typename Word>
		void WriteWord(Word word, TOStream& stream, bool requireIO=true) {
			RequireIO rio{stream, requireIO};
			if constexpr(sizeof word == 1U) {
				stream.write(reinterpret_cast<TStreamByte*>(&word), 1);
			}
			else if constexpr(sizeof word > 1U) {
				std::array<std::byte, sizeof word> buffer;
				WriteWord(word, buffer.data());
				stream.write(
					reinterpret_cast<TStreamByte*>(buffer.data()),
					sizeof word
				);
			}
		}
	template<typename Word>
		auto ReadWord(TIStream& stream, bool requireIO=true) -> Word {
			Word word{};
			RequireIO rio{stream, requireIO};
			if constexpr(sizeof word == 1U) {
				stream.read(reinterpret_cast<TStreamByte*>(&word), 1);
			}
			else if constexpr(sizeof word > 1U) {
				std::array<std::byte, sizeof word> buffer;
				stream.read(
					reinterpret_cast<TStreamByte*>(buffer.data()),
					sizeof word
				);
				word = ReadWord<Word>(buffer.data());
			}
			return word;
		}
}

#endif
