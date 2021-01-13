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
		typename I: inferred from function i arg

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
		typename Word: inferred from function word arg
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
		typename Word: see WriteWord byte buffer variant

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
		typename Word: word type to read back

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
		typename Word: word type to read back

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
	PackedBoolsGenData struct template

	This is the ancillary data type associated with Generator returned by the
	PackedBoolsGen template function. It contains the boolean iterators used
	internally, but of more interest is the mBoolCount field. If you read
	myGen->mBoolCount after your iteration loop, it should indicate how many
	bools were packed into the bytes: a useful number to know later when you
	call UnpackedBoolsGen.
	**/
	template<typename BoolIt, typename EndIt>
	struct PackedBoolsGenData {
		BoolIt mBoolIt;
		EndIt mEndIt;
		std::size_t mBoolCount;
	};
	/**
	PackedBoolsGen function template

	This function returns a generator that produced bytes from bools, with up to
	8 bools packed into each byte.

	Template Args:
		typename BoolIt: inferred from boolIt function arg
		typename EndIt: inferred from endIt function arg

	Args:
		boolIt (BoolIt): input iterator to bool values
			This iterator will only be accessed sequentially until endIt is
			reached. (Technically, PackedBoolsGen can work with any value type
			that can evaluate as bool in a conditional expression.)
		endIt (EndIt): suitable end iterator for boolIt

	Returns:
		Generator of std::byte:
			This is an iterable to a series of bytes generated from the input
			bools. Each byte contains up to 8 bools packed into its bits. The
			packing algorithm follows a big-endian bit order. In other words,
			the first bool goes into the most-significant bit.

			If there are not enough bools to fill the final byte, the remaining
			least-significant bits will be cleared.

			Note that you can read myGen->mBoolCount to check how many bools
			were packed into the bytes.
	**/
	template<typename BoolIt, typename EndIt>
		auto PackedBoolsGen(BoolIt boolIt, const EndIt& endIt) {
			return MakeGenerator<PackedBoolsGenData<BoolIt,EndIt>>(
				[](PackedBoolsGenData<BoolIt,EndIt>& data) {
					auto& [boolIt, endIt, boolCnt] = data;
					auto byteVal = 0x00u;
					auto i = 0u;
					for(; boolIt != endIt && i < 8u; ++boolIt, ++i) {
						byteVal <<= 1;
						byteVal |= *boolIt ? 0x01u : 0x00u;
					}
					if(i < 8u) {
						byteVal <<= 8u - i;
					}
					boolCnt += i;
					return MakeOpt(i, ToByte(byteVal));
				}, boolIt, endIt, 0u);
		}

	/**
	UnpackedBoolsGen function template

	This function takes bools previously packed into bytes by PackedBoolsGen and
	returns a generator that reproduces those bools.

	Template Args:
		typename ByteIt: inferred fromo byteIt function arg

	Args:
		byteIt (ByteIt): an input iterator with a value_type of std::byte
		boolCnt (std::size_t): number of bools to unpack

	Returns:
		Generator of bool
	**/
	template<typename ByteIt>
	auto UnpackedBoolsGen(ByteIt byteIt, std::size_t boolCnt) {
		decltype(boolCnt) i = 0u;
		auto byteVal = 0x00u;
		return MakeGenerator(
			[byteIt, boolCnt, i, byteVal]() mutable {
				bool gotBool = i < boolCnt;
				if((i++ & 0x7u) == 0x0u && gotBool) {
					byteVal = std::to_integer<decltype(byteVal)>(*byteIt++);
				}
				else { byteVal <<= 1; }
				return MakeOpt(gotBool, (byteVal & 0x80u) != 0x00u);
			});
	}

	/**
	PackBools function template - base variant

	PackBools can be used to pack any number of boolean values into a
	sequence of bytes, with up to 8 bools packed into each byte.

	You provide PackBools with a callback function that returns a single
	bool every time it gets called. PackBools then returns a callback
	function of its own. This function returns a single byte each time
	you call it until the source bools are exhausted.

	For example, say you have a std::vector<bool> called myBools and a
	std::vector<std::byte> called myBytes.

		auto byteGen = PackBools(
			[&myBools](std::size_t i) { return myBools[i]; },
			myBools.size()
			);
		myBytes.clear();
		for (auto optByte = byteGen(); optByte; optByte = byteGen()) {
			myBytes.push_back(*optByte);
		}

	These i values are guaranteed to be sequential. That means should see
	i=0 the first time it's called, i=1 the second time, and so on until
	the final call with i=myBools.size()-1. That means you could go with
	a sequential iterator instead:

		auto iter = myBools.begin();
		auto byteGen = PackBools(
			[&iter](std::size_t) { return *iter++; },
			myBools.size());
		// the rest is the same...

	This form in which we ignore i would be better suited to packing a
	std::list<bool>, though it would work for a vector also.

	Packed Data Format:
		Boolean values are packed 8 to a byte in order starting from the
		most-signficant bit down to the least. If there are fewer than 8
		values remaining to pack, the unused least-significant bits are
		left cleared by convention.

		In other words, the format is big-endian at the bit level.

	Template Args:
		typename BoolGen: inferred from function boolGen arg

	Args:
		boolGen (function): callback that returns each boolean value
			This function should take the form:

				bool boolGen(std::size_t i)

			Here, i is the index of a particular bool and args are any
			extra arguments you want to supply to your function.

			Note that PackBools guarantees sequential access. That means
			your generator will get called boolCnt times with indices in
			order from 0 to boolCnt-1. The guarantee implies that you can
			use a sequential iterator to generate your bools (likely
			ignoring i in that case). In other words, your container need
			not support random access (though it does require its size be
			known right from the start).

			(While the return type is typically bool, it can technically be
			anything that can evaluate as a boolean in a conditional
			expression.)
		boolCnt (std::size_t): number of bools to pack

	Returns:
		function:
			This function takes the form:

				std::optional<std::byte> function()

			After (boolCnt + 7) / 8 calls to this function, it will stop
			supplying the optional byte value.
	**/
	template<typename BoolGen>
		auto PackBools(BoolGen boolGen, std::size_t boolCnt) {
			decltype(boolCnt) i = 0u;
			return [boolGen, boolCnt, i]() mutable {
				std::optional<std::byte> optByte;
				if(i < boolCnt) {
					auto byt = 0x00_byte;
					decltype(boolCnt) iPlus8 = i + 8u;
					decltype(boolCnt) n = std::min(boolCnt, iPlus8);
					for(; i < n; ++i) {
						byt <<= 1;
						byt |= boolGen(i)
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
	/**
	PackBools function template - stream variant

	This version of PackBools writes the generated bytes to a binary output
	stream instead of simply returning them to you.

	Template Args:
		typename BoolGen: inferred from function BoolGen arg

	Args:
		boolGen (function): see previous PackBools base variant
		boolCnt (std::size_t): see previous PackBools base variant
		stream (TOStream): output binary stream for packed bytes
		requireIO (bool, optional): temporarily sets stream exception flags
			If anything goes wrong accessing stream, an exception will be thrown
			if these flags are set. You can pass kSkipRequireIO (false), which
			will leave the flags untouched. (They may already be set, however.)

			Note that if requireIO is true, the flags will be restored back to
			their original values before the function returns.

	Returns:
		std::size_t: the number of bytes written to the stream
	**/
	template<typename BoolGen>
		auto PackBools(BoolGen boolGen, std::size_t boolCnt,
			TOStream& stream, bool requireIO=true) -> std::size_t
		{
			RequireIO rio{stream, requireIO};
			auto byteGen = PackBools(boolGen, boolCnt);
			std::size_t byteCnt = 0;
			auto optByte = byteGen();
			for(; optByte; optByte = byteGen(), ++byteCnt) {
				stream.write(reinterpret_cast<TStreamByte*>(&*optByte), 1);
			}
			return byteCnt;
		}

	/**
	UnpackBools function template - base variant

	UnpackBools can be used to unpack the boolean values packed into bytes
	using PackBools earlier. In this case, your callback returns bytes and
	UnpackBools' callback returns bools.

	Going back to the first example under PackBools, you could unpack your
	byte-packed bools with:

		auto boolGen = UnpackBools(
			[&myBytes](std::size_t i) { return myBytes[i]; },
			myBools.size()
			);
		myBools.clear();
		for (auto optBool = boolGen(); optBool; optBool = boolGen()) {
			myBools.push_back(*optBool);
		}

	Template Args:
		typename ByteGen: inferred from byteGen function arg

	Args:
		byteGen (function): callback that returns each byte value
			This function should take the form:

				std::byte byteGen(std::size_t i)

			Here, i is the index of a particular byte and args are any
			extra arguments you want to supply to your function.

			As with PackBools, UnpackBools has a sequential access
			guarantee, though in this case it counts through the range
			[0, (boolCnt + 7) / 8 - 1] since we are counting bytes rather
			than bits.
		boolCnt: number of bools to unpack

	Returns:
		function:
			This function takes the form:

				std::optional<bool> function()

			After boolCnt calls to this function, it will stop supplying
			the optional bool value.
	**/
	template<typename ByteGen>
		auto UnpackBools(ByteGen byteGen, std::size_t boolCnt)
		{
			auto byt = 0x00_byte;
			decltype(boolCnt) i = 0u;
			return [byteGen, boolCnt, byt, i]() mutable {
				std::optional<bool> optBool;
				if(i < boolCnt) {
					if((i & 0x7) == 0x0) {
						byt = byteGen(i >> 3);
					}
					optBool = std::make_optional(
						(byt & 0x80_byte) != 0x00_byte);
					byt <<= 1;
					++i;
				}
				return optBool;
			};
		}
	/**
	UnpackBools function - stream variant

	This variant of UnpackBools reads bytes from a binary input stream rather
	than from a byte generator you supply.

	Args:
		boolCnt (std::size_t): see previous PackBools base variant
		stream (TIStream): input binary stream source of packed bytes
		requireIO (bool, optional): see PackBools stream variant

	Returns:
		function: see UnpackBools base variant
	**/
	auto UnpackBools(std::size_t boolCnt,
		TIStream& stream, bool requireIO=true);
}

#endif
