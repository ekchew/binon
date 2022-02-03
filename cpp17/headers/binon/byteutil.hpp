#ifndef BINON_BYTEUTIL_HPP
#define BINON_BYTEUTIL_HPP

#include "ioutil.hpp"
#include "literals.hpp"

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#if BINON_CPP20
	#include <bit>
#endif

namespace binon {
	static_assert(CHAR_BIT == 8, "binon requires 8-bit bytes");

	//-------------------------------------------------------------------------
	//
	//	std::byte Helper Functions
	//
	//-------------------------------------------------------------------------

	//	ToByte function template
	//
	//	While you can easily convert a std::byte to an integer by calling
	//
	//		std::to_integer<int>(myByte)
	//
	//	there does not appear to be a corresponding std::to_byte function, so
	//	here is an alternative.
	//
	//	Template Args:
	//		I (type, inferred)
	//
	//	Args:
	//		i (integral type): value to convert to std::byte assertRange (bool,
	//		optional): make sure i fits in a byte?
	//			This option defaults to true only in debug builds. When true,
	//			ToByte may throw a std::out_of_range if i's value lies outside
	//			of what can be represented by a byte. To put it another way, i
	//			must lie in the range [-128,255].
	//
	//	Returns:
	//		std::byte: the byte form of i
	template<typename I> constexpr auto ToByte(
		I i, bool assertRange=BINON_DEBUG
		) -> std::byte
		{
			if(	assertRange &&
				static_cast<std::make_unsigned_t<I>>(i) >= 0x100u)
			{
				throw std::out_of_range{"int to byte conversion loses data"};
			}
			return std::byte{static_cast<unsigned char>(i)};
		}

	//	AsHexC function
	//
	//	This is a low-level function to convert a std::byte into a zero-padded
	//	hexadecimal C string.
	//
	//	Args:
	//		value (std::byte): byte to convert capitalize (bool, optional):
	//		return "AB" rather than "ab"?
	//			Defaults to false.
	//
	//	Returns:
	//		std::array<char,3>: 2 hexadecimal digits followed by null terminator
	auto AsHexC(std::byte value, bool capitalize=false) noexcept
		-> std::array<char,3>;

	//	AsHex function
	//
	//	This is like AsHexC except it returns a std::string. This might be
	//	somewhat less efficient, though thanks to the small string optimization,
	//	it may not necessarily require any dynamic allocation at least.
	//
	//	Args:
	//		value (std::byte): byte to convert capitalize (bool, optional):
	//		return "AB" rather than "ab"?
	//			Defaults to false.
	//
	//	Returns:
	//		std::string: 2 hexadecimal digits
	auto AsHex(std::byte value, bool capitalize=false) -> std::string;

	//	PrintByte function
	//
	//	This function prints a std::byte as 2 hexadecimal digits to a text
	//	stream.
	//
	//	Args:
	//		value (std::byte): byte to print stream (std::ostream&):
	//			target text output stream
	//		capitalize (bool, optional): return "AB" rather than "ab"?
	//			Defaults to false.
	void PrintByte(std::byte value, std::ostream& stream,
		bool capitalize=false);

	//-------------------------------------------------------------------------
	//
	//	Byte Order Functions
	//
	//-------------------------------------------------------------------------

	//	LittleEndian function
	//
	//	C++20 can determine the byte order convention at compile time, while
	//	C++17 requires some runtime logic (unless you look at precompiler macros
	//	which would be compiler-dependent -- the current implementation does not
	//	do so).
	//
	//	Returns:
	//		bool: compiler target uses little-endian byte order?
#if BINON_CPP20
	constexpr auto LittleEndian() noexcept -> bool {
		return std::endian::native == std::endian::little;
	}
 #else
	auto LittleEndian() noexcept -> bool;
 #endif

	//	WriteWord function template - byte buffer variant
	//
	//	Writes a scalar value such as an integer into a byte buffer following a
	//	big-endian byte order.
	//
	//	Template Args:
	//		Word (type, inferred)
	//			You are strongly advised to use a type with a rigorously-defined
	//			size such as std::int32_t or binon::types::TFloat64 for code
	//			portability's sake.
	//
	//	Args:
	//		word (Word): value to serialize buffer (std::byte*): base address of
	//		a byte buffer
	//			The buffer must have a minimum capacity of sizeof(Word) bytes.
	//
	//	Returns:
	//		std::byte*: buffer
	//			This is the same address you input, but of course now it should
	//			be pointing at your serialized word data.
	template<typename Word>
		auto WriteWord(Word word, std::byte* buffer) noexcept -> std::byte* {
			if constexpr(sizeof word == 1U) {
				buffer[0] = reinterpret_cast<std::byte&>(word);
			}
			else if constexpr(sizeof word > 1U) {
				std::memcpy(buffer, &word, sizeof(Word));
				if BINON_IF_CPP20(constexpr)(LittleEndian()) {
					std::reverse(buffer, buffer + sizeof(Word));
				}
			}
			return buffer;
		}

	//	WriteWord function template - stream variant
	//
	//	In this case, your word is written to a binary output stream instead of
	//	a byte buffer.
	//
	//	Template Args:
	//		Word (type, inferred)
	//
	//	Args:
	//		word (Word): see WriteWOrd byte buffer variant
	//		stream (TOStream):
	//			binary output stream
	//		requireIO (bool, optional): set std::ios exception bits?
	//			See also RequireIO class in ioutil.hpp.
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

	//	ReadWord function template - byte buffer variant
	//
	//	Reads back the word you wrote into a byte buffer with WriteWord.
	//
	//	Template Args:
	//		Word (type, required): word type to read back
	//
	//	Args:
	//		buffer (const std::byte*): base address of byte buffer
	//
	//	Returns:
	//		Word: reconstituted word value
	template<typename Word>
		auto ReadWord(const std::byte* buffer) noexcept -> Word {
			Word word;
			if constexpr(sizeof word == 1U) {
				word = reinterpret_cast<const Word*>(buffer)[0];
			}
			else if constexpr(sizeof word > 1U) {
				std::memcpy(&word, buffer, sizeof word);
				if BINON_IF_CPP20(constexpr)(LittleEndian()) {
					auto p = reinterpret_cast<std::byte*>(&word);
					std::reverse(p, p + sizeof(Word));
				}
			}
			return word;
		}

	//	ReadWord function template - stream variant
	//
	//	Template Args:
	//		Word (type, required): word type to read back
	//
	//	Args:
	//		stream (TIStream): binary input stream requireIO (bool, optional):
	//		requireIO (bool, optional): set std::ios exception bits?
	//			See also RequireIO class in ioutil.hpp.
	//
	//		Returns:
	//			Word: reconstituted word value
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
