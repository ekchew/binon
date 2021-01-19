#ifndef BINON_BYTEUTIL_HPP
#define BINON_BYTEUTIL_HPP

#include "generator.hpp"
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

	/**
	ToByte function template

	While you can easily convert a std::byte to an integer by calling

		std::to_integer<int>(myByte)

	there does not appear to be a corresponding std::to_byte function, so here
	is an alternative.

	Template Args:
		I (type, inferred)

	Args:
		i (integral type): value to convert to std::byte
		assertRange (bool, optional): make sure i fits in a byte?
			This option defaults to true only in debug builds. When true,
			ToByte may throw a std::out_of_range if i's value lies outside of
			what can be represented by a byte. To put it another way, i must lie
			in the range [-0x80,0xff].

	Returns:
		std::byte: the byte form of i
	**/
	template<typename I> constexpr auto ToByte(
		I i, bool assertRange=BINON_DEBUG
		) {
			if(	assertRange &&
				static_cast<std::make_unsigned_t<I>>(i) >= 0x100u)
			{
				throw std::out_of_range{"int to byte conversion loses data"};
			}
			return std::byte{static_cast<unsigned char>(i)};
		}

	/**
	AsHexC function

	This is a low-level function to convert a std::byte into a zero-padded
	hexadecimal C string.

	Args:
		value (std::byte): byte to convert
		capitalize (bool, optional): return "AB" rather than "ab"?
			Defaults to false.

	Returns:
		std::array<char,3>: 2 hexadecimal digits followed by null terminator
	**/
	auto AsHexC(std::byte value, bool capitalize=false) noexcept
		-> std::array<char,3>;

	/**
	AsHex function

	This is like AsHexC except it returns a std::string. This may require some
	dynamic allocation which the former avoids, but tends to be easier to use.

	Args:
		value (std::byte): byte to convert
		capitalize (bool, optional): return "AB" rather than "ab"?
			Defaults to false.

	Returns:
		std::string: 2 hexadecimal digits
	**/
	auto AsHex(std::byte value, bool capitalize=false) -> std::string;

	/**
	PrintByte function

	This function prints a std::byte as 2 hexadecimal digits to a text stream.

	Args:
		value (std::byte): byte to print
		stream (std::ostream&): target text output stream
		capitalize (bool, optional): return "AB" rather than "ab"?
			Defaults to false.
	**/
	void PrintByte(std::byte value, std::ostream& stream,
		bool capitalize=false);

	//-------------------------------------------------------------------------
	//
	//	Byte Order Functions
	//
	//-------------------------------------------------------------------------

	/**
	LittleEndian function

	C++20 can determine the byte order convention at compile time, while C++17
	requires some runtime logic (unless you look at precompiler macros which
	would be compiler-dependent -- the current implementation does not do so).

	Returns:
		bool: compiler target uses little-endian byte order?
	**/
#if BINON_CPP20
	constexpr auto LittleEndian() noexcept -> bool {
		return std::endian::native == std::endian::little;
	}
#else
	auto LittleEndian() noexcept -> bool;
#endif

	/**
	WriteWord function template - byte buffer variant

	Writes a scalar value such as an integer into a byte buffer following using
	a big-endian byte order (regardless of what the compiler target prefers).

	Template Args:
		Word (type, inferred)
			You are strongly advised to use a type with a rigorously-defined
			size such as std::int32_t or binon::types::TFloat64 for code
			portability's sake.

	Args:
		word (Word): value to serialize
		buffer (std::byte*): base address of a byte buffer
			The buffer must have a minimum capacity of sizeof(Word) bytes.

	Returns:
		std::byte*: buffer
			This is the same address you input, but of course now it should be
			pointing at your serialized word data.
	**/
	template<typename Word>
		auto WriteWord(Word word, std::byte* buffer) noexcept {
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
	/**
	WriteWord function template - stream variant

	In this case, your word is written to a binary output stream instead of a
	byte buffer.

	Template Args:
		Word (type, inferred)

	Args:
		word (Word): see WriteWOrd byte buffer variant
		stream (TOStream): binary output stream
		requireIO (bool, optional): see PackBools - stream variant
	**/
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

	/**
	ReadWord function template - byte buffer variant

	Reads back the word you wrote into a byte buffer with WriteWord.

	Template Args:
		Word (type, required): word type to read back

	Args:
		buffer (const std::byte*): base address of byte buffer

	Returns:
		Word: reconstituted word value
	**/
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
	/**
	ReadWord function template - stream variant

	Template Args:
		Word (type, required): word type to read back

	Args:
		stream (TIStream): binary input stream
		requireIO (bool, optional): see PackBools - stream variant

		Returns:
			Word: reconstituted word value
	**/
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

	//-------------------------------------------------------------------------
	//
	//	Bool-Packing Functions
	//
	//-------------------------------------------------------------------------

	/**
	PackedBoolsGen function template

	This function returns a generator that produced bytes from bools, with up to
	8 bools packed into each byte.

	Template Args:
		BoolGen (type, inferred)

	Args:
		boolGen (BoolGen): Generator of bool or bool-like values
			Technically, any type that can evaluate as boolean for the purposes
			of an if statement should work here.

	Returns:
		Generator of std::byte:
			This is an iterable to a series of bytes generated from the input
			bools. Each byte contains up to 8 bools packed into its bits. The
			packing algorithm follows a big-endian bit order. In other words,
			the first bool goes into the most-significant bit.

			If there are not enough bools to fill the final byte, the remaining
			least-significant bits will be cleared.

			Note that you can dereference the returned Generator (*myGen) after
			your iteration loop to see how many bools were packed. (This would
			be useful to know when you eventually call UnpackedBoolsGen.)
	**/
	template<typename BoolGen>
		auto PackedBoolsGen(BoolGen boolGen) {
			auto byteGen = [](auto& gen, auto& it, auto& n) {
				auto byt = 0x00_byte;
				auto i = 0u;
				for(; it != gen.end() && i < 8u; ++it, ++i) {
					byt <<= 1;
					byt |= *it ? 0x01_byte : 0x00_byte;
				}
				if(i < 8u) {
					byt <<= 8u - i;
				}
				n += i;
				return MakeOpt<std::byte>(i, [byt] { return byt; });
			};
			return ChainGenerators<std::byte,std::size_t>(boolGen, byteGen, 0u);
		}
	template<typename BoolIt, typename EndIt>
		auto PackedBoolsGen(BoolIt boolIt, EndIt endIt) {
			return PackedBoolsGen(MakeIterGen(boolIt, endIt));
		}

	/**
	UnpackedBoolsGen function template

	This function takes bools previously packed into bytes by PackedBoolsGen and
	returns a generator that reproduces those bools.

	Template Args:
		ByteIt (type, inferred)

	Args:
		byteIt (ByteIt): an input iterator with a value_type of std::byte
		boolCnt (std::size_t): number of bools to unpack

	Returns:
		Generator of bool
	**/
	template<typename ByteIt>
		auto UnpackedBoolsGen(ByteIt byteIt, std::size_t boolCnt) {
			auto nextBool = [](auto& byteIt, auto boolCnt, auto i, auto& byt) {
				if((i & 0x7u) == 0x0u) {
					byt = i > 0u ? *++byteIt : *byteIt;
				}
				else { byt <<= 1; }
				return (byt & 0x80_byte) != 0x00_byte;
			};
			decltype(boolCnt) i = 0u;
			auto byt = 0x00_byte;
			return MakeGenerator<bool>(
				[byteIt, boolCnt, nextBool, i, byt]() mutable {
					++i;
					return MakeOpt<bool>(
						i <= boolCnt, nextBool,
						byteIt, boolCnt, i - 1u, byt);
				});
		}
	/*template<typename ByteGen>
		auto UnpackedBoolsGen(ByteGen byteGen, std::size_t boolCnt) {
			using Size = decltype(boolCnt);
			auto byt = 0x00_byte;
			auto nextBool = [byt](auto& it, Size i) mutable {
				if((i & 0x7u) == 0x0u) {
					byt = i > 0u ? *++it : *it;
				}
				else { byt <<= 1; }
				return (byt & 0x80_byte) != 0x00_byte;
			};
			Size i = 0u;
			auto nextOptBool = [boolCnt, i](auto&, auto& it) mutable {
				auto j = i++;
				return MakeOpt<bool>(j < boolCnt, nextBool, it, j);
			};
			return ChainGenerator<bool>(byteGen, nextOptBool);
		}
	*/
}

#endif
