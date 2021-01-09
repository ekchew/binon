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
#include <string>
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

	//	Prints a byte in the literal form "0xa5_byte".
	void PrintByte(std::byte value, std::ostream& stream);

	//-------------------------------------------------------------------------
	//
	//	Byte Order Functions
	//
	//-------------------------------------------------------------------------

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
				if BINON_IF_CPP20(constexpr)(LittleEndian()) {
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
				if BINON_IF_CPP20(constexpr)(LittleEndian()) {
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

	//-------------------------------------------------------------------------
	//
	//	Bool-Packing Functions
	//
	//-------------------------------------------------------------------------

	///	PackBools function template
	//
	//	This function packs 8 bools to a byte. Its return value is a lambda
	//	function which, in turn, returns the byte data one byte at a time.
	//
	//	The bools are packed starting from the most-significant bit in each
	//	byte. If you are not packing a multiple of 8 bools, the least-
	//	significant bits of the final byte will be left cleared.
	//
	//	Template Args:
	//		typename BoolGen: inferred from function args
	//		typename... Args: inferred from function args
	//
	//	Args:
	//		boolGen (function): function that returns each boolean value
	//			This function should take the form:
	//
	//				bool boolGen(std::size_t i, Args... args)
	//
	//			Here, i is the index of a particular bool and args are any
	//			extra arguments you want to supply to your function.
	//
	//			Note that PackBools guarantees sequential access. That means
	//			your generator will get called boolCnt times with indices in
	//			order from 0 to boolCnt - 1. The guarantee implies that you
	//			can use a sequential iterator to generate your bools (likely
	//			ignoring i in that case). You need not support random access.
	//
	//			While the return value is typically a bool, it can technically
	//			be anything that evaluates as a boolean in a conditional
	//			expression.
	//		boolCnt (std::size_t): number of bools to pack
	//		args... (any types): extra args for your boolGen function
	//
	//	Returns:
	//		function:
	//			This function takes the form:
	//
	//				std::optional<std::byte> function()
	//
	//			After (boolCnt + 7) / 8 calls to this function, it will stop
	//			supplying the optional byte value.
	///
	template<typename BoolGen, typename... Args>
		auto PackBools(BoolGen boolGen, std::size_t boolCnt, Args&&... args) {
			decltype(boolCnt) i = 0u;
			return [boolGen, boolCnt, &args..., i]() mutable {
				std::optional<std::byte> optByte;
				if(i < boolCnt) {
					auto byt = 0x00_byte;
					decltype(boolCnt) iPlus8 = i + 8u;
					decltype(boolCnt) n = std::min(boolCnt, iPlus8);
					for(; i < n; ++i) {
						byt <<= 1;
						byt |= boolGen(i, std::forward<Args>(args)...)
							? 0x01_byte : 0x00_byte;
					}
					if(iPlus8 > boolCnt) {
						byt <<= iPlus8 - boolCnt;
					}
					optByte = std::make_optional(byt);
				}
				return optByte;
			};
		}

	///	UnpackBools function template
	//
	//	This function unpacks bool values previously packed into bytes by the
	//	PackBools function.
	//
	//	Template Args:
	//		typename ByteGen: inferred from function args
	//		typename... Args: inferred from function args
	//
	//	Args:
	//		byteGen (function): function that returns each byte value
	//			This function should take the form:
	//
	//				std::byte byteGen(std::size_t i, Args... args)
	//
	//			Here, i is the index of a particular byte and args are any
	//			extra arguments you want to supply to your function.
	//
	//			As with PackBools, UnpackBools has a sequential access
	//			guarantee, though in this case it counts through the range
	//			[0, (boolCnt + 7) / 8 - 1] since we are counting bytes rather
	//			than bits.
	//		boolCnt: number of bools to unpack
	//		args... (any types): extra args for your byteGen function
	//
	//	Returns:
	//		function:
	//			This function takes the form:
	//
	//				std::optional<bool> function()
	//
	//			After boolCnt calls to this function, it will stop supplying
	//			the optional bool value.
	///
	template<typename ByteGen, typename... Args>
		auto UnpackBools(ByteGen byteGen, std::size_t boolCnt, Args&&... args)
		{
			auto byt = 0x00_byte;
			decltype(boolCnt) i = 0u;
			return [byteGen, boolCnt, &args..., byt, i]() mutable {
				std::optional<bool> optBool;
				if(i < boolCnt) {
					if((i & 0x7) == 0x0) {
						byt = byteGen(i >> 3, std::forward<Args>(args)...);
					}
					optBool = std::make_optional(
						(byt & 0x80_byte) != 0x00_byte);
					byt <<= 1;
					++i;
				}
				return optBool;
			};
		}
}

#endif
