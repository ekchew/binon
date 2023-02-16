#ifndef BINON_BYTEUTIL_HPP
#define BINON_BYTEUTIL_HPP

#include "errors.hpp"
#include "floattypes.hpp"
#include "ioutil.hpp"
#include "typeutil.hpp"

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <optional>
#include <limits>
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
	//	_byte Literal
	//
	//	std::byte's implementation does not seem to include a format for byte
	//	literals. So with this user-defined literal, you can now write 0x42_byte
	//	instead of std::byte{0x42}.
	//
	//	It is currently the only definition within the binon::literals
	//	namespace.
	//
	//	Note that you will likely see a ByteTrunc exception at compile time if
	//	you try to specify a byte greater than 0xff_byte.
	//
	//-------------------------------------------------------------------------

	inline namespace literals {
		constexpr auto operator ""_byte(unsigned long long i) {
				if(i > std::numeric_limits<std::uint8_t>::max()) {
					throw ByteTrunc{"_byte literal out of range"};
				}
				return std::byte{static_cast<unsigned char>(i)};
			}
	}

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
	//			ToByte may throw a ByteTrunc if i's value lies outside
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
				throw ByteTrunc{"int to byte conversion loses data"};
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

	//-------------------------------------------------------------------------
	//
	//	Byte-Like Type Handling
	//
	//	Byte-like types include std::byte as well as any single-byte integral
	//	types such as char.
	//
	//-------------------------------------------------------------------------

	//	ConvByteLike function:
	//		Converts a value from one byte-like type to another.
	template<typename To, typename From> constexpr
		auto ConvByteLike(From v) noexcept -> To {
			static_assert(sizeof(From) == 1U and sizeof(To) == 1U);
			using std::byte, std::is_same_v;
			if constexpr(is_same_v<From, byte>) {
				if constexpr(is_same_v<To, byte>) {
					return v;
				}
				else {
					return std::to_integer<To>(v);
				}
			}
			else
				if constexpr(is_same_v<To, byte>) {
					return std::byte{
						static_cast<std::underlying_type_t<byte>>(v)
						};
				}
				else {
					return static_cast<To>(v);
				}
		}

	//-------------------------------------------------------------------------
	//
	//	Serialization
	//
	//	The functions defined here help serialize fundamental data types
	//	(integral or floating-point) into byte sequences and vice versa.
	//
	//	For example, you could encode the integer 1000 into 2 bytes with:
	//
	//		auto arr = BytePack<2>(1000);
	//
	//	Here, arr will be a std::array<TStreamByte,2> containing:
	//
	//		{'\x03','\xf8'}
	//
	//	As you can see, serialization follows a big-endian byte ordering
	//	convention.
	//
	//	Later, you can unpack the serialized bytes with:
	//
	//		auto i = ByteUnpack<int,2>(arr);  // i = 1000
	//
	//	There are also WriteAsBytes() and ReadAsBytes() functions that
	//	serialize data to/from TOStream and TIStream streams, respectively.
	//
	//	Regarding floating-point values, BinON currently only supports IEEE 754
	//	binary32 and binary64 formats, so the size needs to be either 4 or 8
	//	bytes. binon::types::TFloat32 and binon::types::TFloat64 are meant to
	//	map onto these two data types.
	//
	//-------------------------------------------------------------------------

	//	kBytePackSize constant:
	//		This evaluates how large the std::array returned by BytePack()
	//		should be dimensioned.
	template<typename T, std::size_t Size>
		inline constexpr std::size_t kBytePackSize = Size ? Size : sizeof(T);

	namespace details {
		template<std::size_t Size=0, typename Byte=TStreamByte, typename Int>
			auto BytePackInt(Int i) noexcept
				-> std::array<Byte, kBytePackSize<Int, Size>>
			{
				static_assert(std::is_integral_v<Int>);
				using std::array;
				using UC = unsigned char;
				constexpr std::size_t N = kBytePackSize<Int, Size>;
				if constexpr(N == 1U) {
					return std::array<Byte, N>{ConvByteLike<Byte>(i)};
				}
				else {
					std::array<Byte, N> arr;
					auto end = arr.rend();
					for(auto it = arr.rbegin(); it != end; ++it, i >>= 8) {
						*it = ConvByteLike<Byte>(static_cast<UC>(i & 0xffU));
					}
					return arr;
				}
			}

		template<std::size_t Size=0, typename Byte=TStreamByte, typename Flt>
			auto BytePackFloat(Flt x) noexcept
				-> std::array<Byte, kBytePackSize<Flt, Size>>
			{
				static_assert(std::is_floating_point_v<Flt>);
				using std::array;
				constexpr std::size_t N = kBytePackSize<Flt, Size>;
				static_assert(N == 4 or N == 8,
					"BinON expects binary32 or binary64 floating-point types");
				auto pack = [](auto x) noexcept -> array<Byte, sizeof x> {
					array<Byte, sizeof x> arr;
					std::memcpy(arr.data(), &x, sizeof x);
					if(LittleEndian()) {
						std::reverse(arr.begin(), arr.end());
					}
					return arr;
				};
				if constexpr(N == 4) {
					return pack(static_cast<types::TFloat32>(x));
				}
				else {
					return pack(static_cast<types::TFloat64>(x));
				}
			}

		template<typename UInt, typename Byte, std::size_t Size>
			constexpr auto ByteUnpackUInt(const std::array<Byte, Size>& arr)
				noexcept -> UInt
			{
				static_assert(std::is_unsigned_v<UInt>);
				if constexpr(Size == 1U) {
					return ConvByteLike<unsigned char>(arr[0]);
				}
				else {
					UInt i{};
					for(auto b: arr) {
						i <<= 8;
						i |= ConvByteLike<unsigned char>(b);
					}
					return i;
				}
			}

		template<typename SInt, typename Byte, std::size_t Size>
			auto ByteUnpackSInt(const std::array<Byte, Size>& arr) noexcept
				-> SInt
			{
				static_assert(
					std::is_integral_v<SInt> and not std::is_unsigned_v<SInt>);
				using UInt = std::make_unsigned_t<SInt>;
				UInt u = ByteUnpackUInt<UInt, Byte, Size>(arr);
				if constexpr(Size >= sizeof(SInt)) {
					return static_cast<SInt>(u);
				}
				else {
					UInt m = UInt{1} << (Size * 8U - 1U);
					SInt v = static_cast<SInt>(u & (m - 1U));
					if(u & m) {
						v -= static_cast<SInt>(m);
					}
					return v;
				}
			}

		template<typename Flt, typename Byte, std::size_t Size>
			auto ByteUnpackFloat(std::array<Byte, Size> arr) noexcept -> Flt {
				static_assert(
					std::is_floating_point_v<Flt> and (Size == 4 or Size == 8),
					"BinON expects binary32 or binary64 floating-point types");
				if(LittleEndian()) {
					std::reverse(arr.begin(), arr.end());
				}
				if constexpr(Size == 4) {
					types::TFloat32 x;
					memcpy(&x, arr.data(), Size);
					return static_cast<Flt>(x);
				}
				else {
					types::TFloat64 x;
					memcpy(&x, arr.data(), Size);
					return static_cast<Flt>(x);
				}
			}
	}

	//	BytePack function -- array version
	//
	//	Packs a value into an array of bytes. (Note that this is the
	//	lowest-level version of BytePack. All overloads call this internally.)
	//
	//	Template args:
	//		Size: number of bytes to pack or the default 0
	//			In the 0 case, the number of bytes is determined from the size
	//			of the type T. (This works best if you use a type with of well-
	//			defined size such as std::int32_t.)
	//
	//			For integral types, if Size is set too small to accommodate the
	//			entire value, the most-significant bytes will be discarded.
	//
	//			For floating-point types, the size must be either 4 or 8, since
	//			BinON only supports binary32 and binary64 values.
	//		Byte: a byte-like type (defaults to TStreamByte)
	//			This is the element type of the returned std::array.
	//		T (inferred from function arg "v"): type of value to pack
	//			This can be an integral type, floating-point type,
	//			or std::byte.
	//
	//	Function args:
	//		v: the value to pack
	//
	//	Returns:
	//		a std::array of Byte elements containing the packed "v" value
	//
	template<std::size_t Size=0, typename Byte=TStreamByte, typename T>
		auto BytePack(T v) noexcept -> std::array<Byte, kBytePackSize<T, Size>>
		{
			if constexpr(std::is_floating_point_v<T>) {
				return details::BytePackFloat<Size, Byte, T>(v);
			}
			else if constexpr(std::is_integral_v<T>) {
				return details::BytePackInt<Size, Byte, T>(v);
			}
			else {
				static_assert(std::is_same_v<T, std::byte> and Size <= 1U);
				return std::array<Byte, 1>{ConvByteLike<Byte>(v)};
			}
		}

	//	BytePack function -- iterator version
	//
	//	This overloaded version of BytePack writes the byte data to an output
	//	iterator rather than returning it in a std::array.
	//
	//	Template args:
	//		Size: see array version
	//		T (inferred): see array version
	//		OutputIt (inferred from "it" function arg):
	//			This should be an output iterator to a byte-like type.
	//
	//	Function args:
	//		v: the value to pack
	//		it: output iterator destination for packed bytes
	//
	template<std::size_t Size=0, typename T, typename OutputIt>
		auto BytePack(T v, OutputIt it) noexcept
			-> std::enable_if_t<kIsIterator<OutputIt>>
		{
			using Byte = typename std::iterator_traits<OutputIt>::value_type;
			auto arr = BytePack<Size, Byte, T>(v);
			std::copy(arr.begin(), arr.end(), it);
		}

	//	BytePack function -- stream version
	//
	//	This overloaded version of BytePack writes the byte data to an
	//	output stream rather than returning it in a std::array.
	//
	//	Template args:
	//		Size: see array version
	//		T (inferred): see array version
	//
	//	Function args:
	//		v: the value to stream out
	//		stream: a binary output stream of byte-likes
	//		requireIO: throw exception if write fails? (defaults to true)
	//			Setting this false does not necessarily mean exceptions are
	//			disabled. The stream may have exceptions enabled already?
	//			true will ensure that they are indeed enabled for the duration
	//			of this function before being restored to their original state.
	//
	template<std::size_t Size=0, typename T>
		void BytePack(T v, TOStream& stream, bool requireIO=true) {
			auto arr = BytePack<Size>(v);
			RequireIO rio(stream, requireIO);
			stream.write(arr.data(), arr.size());
		}

	//	ByteUnpack function -- array version
	//
	//	Unpack a numeric value packed into an array earlier by BytePack().
	//	(Note that this is the lowest-level version of ByteUnpack. All
	//	overloads call this internally.)
	//
	//	Template args:
	//		T: type of value to unpack
	//			This may be an integral type, a floating-point type, or
	//			std::byte. (Clearly, you should pick the one that matches
	//			whatever was used to pack the bytes in the first place.)
	//		Byte (inferred from "arr" function arg): array element type
	//			This should a byte-like type.
	//		Size (inferred from "arr" function arg): number of bytes to unpack
	//
	//	Function args:
	//		arr: array of type Byte and size Size containing packed bytes
	//
	//	Returns:
	//		value of type T unpacked from array
	//
	template<typename T, typename Byte, std::size_t Size>
		auto ByteUnpack(const std::array<Byte, Size>& arr) noexcept -> T {
			if constexpr(std::is_floating_point_v<T>) {
				return details::ByteUnpackFloat<T, Byte, Size>(arr);
			}
			else if constexpr(std::is_unsigned_v<T>) {
				return details::ByteUnpackUInt<T, Byte, Size>(arr);
			}
			else if constexpr(std::is_integral_v<T>) {
				return details::ByteUnpackSInt<T, Byte, Size>(arr);
			}
			else {
				static_assert(std::is_same_v<T, std::byte> and Size == 1);
				return ConvByteLike<T>(arr[0]);
			}
		}

	//	ByteUnpack function -- iterator version
	//
	//	This overloaded version of ByteUnpack reads the byte data from an input
	//	iterator rather than taking it from a std::array.
	//
	//	Template args:
	//		T: see array version
	//		Size: number of bytes to unpack (defaults to sizeof(T))
	//		InputIt (inferred from "it" function arg):
	//			This should be an input iterator to a byte-like type.
	//
	//	Function args:
	//		it: input iterator source of the packed bytes
	//
	//	Returns:
	//		value of type T unpacked from the iterator bytes
	//
	template<typename T, std::size_t Size=sizeof(T), typename InputIt>
		auto ByteUnpack(InputIt it) noexcept
			-> std::enable_if_t<kIsIterator<InputIt>, T>
		{
			using Byte = typename std::iterator_traits<InputIt>::value_type;
			std::array<Byte, Size> arr;
			std::copy_n(it, Size, arr.begin());
			return ByteUnpack<T, Byte, Size>(arr);
		}

	//	ByteUnpack function -- stream version
	//
	//	This overloaded version of ByteUnpack reads the byte data from an input
	//	stream rather than taking it from a std::array.
	//
	//	Template args:
	//		T: see array version
	//		Size: number of bytes to unpack (defaults to sizeof(T))
	//
	//	Function args:
	//		stream: a binary input stream of byte-likes
	//		requireIO: throw exception if read fails? (defaults to true)
	//			See note about this arg under BytePack's stream version.
	//			Note that if exceptions are actually disabled, your return
	//			value may be corrupt after an I/O error. You should check the
	//			iostate bit of the stream to determine if the value can be
	//			trusted in that case.
	//
	//	Returns:
	//		value of type T unpacked from the input stream bytes
	//
	template<typename T, std::size_t Size=sizeof(T)>
		auto ByteUnpack(TIStream& stream, bool requireIO=true) -> T {
			std::array<TStreamByte, Size> arr;
			RequireIO rio(stream, requireIO);
			stream.read(arr.data(), arr.size());
			return ByteUnpack<T, TStreamByte, Size>(arr);
		}

	//-------------------------------------------------------------------------
	//
	//	Read/WriteWord functions: DEPRECATED
	//
	//	Use BytePack/Unpack instead.
	//
	//-------------------------------------------------------------------------

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
