#ifndef BINON_BYTEUTIL_HPP
#define BINON_BYTEUTIL_HPP

#include "errors.hpp"
#include "floattypes.hpp"
#include "ioutil.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>

#if !BINON_BIT_CAST
	#include <cstring>
#endif

namespace binon {
	static_assert(
		std::numeric_limits<unsigned char>::digits == 8,
		"BinON requires 8-bit bytes");

	//-------------------------------------------------------------------------
	//
	//	_byte Literal
	//
	//	std::byte's implementation does not seem to include a format for byte
	//	literals. So with this user-defined literal, you can now write
	//	0x42_byte instead of std::byte{0x42}.
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

	//	ToByte function
	//
	//	While you can easily convert a std::byte to an integer by calling
	//
	//		std::to_integer<int>(myByte)
	//
	//	there does not appear to be a corresponding std::to_byte function, so
	//	here is an alternative.
	//
	//	Template args:
	//		AssertRange: check that input value lies in appropriate range?
	//			Defaults to true in debug mode or false otherwise.
	//			An unsigned value should lie in the range [0,255].
	//			A signed value should lie in the range [-128,255].
	//		Int: an integral type (inferred from function arg i)
	//
	//	Function args:
	//		i: the value to convert into a std::byte
	//
	//	Returns:
	//		the std::byte version of i
	//
	template<bool AssertRange=BINON_DEBUG, std::integral Int> constexpr
		auto ToByte(Int i) noexcept(not AssertRange) -> std::byte {
			if constexpr(AssertRange) {
				if(static_cast<std::make_unsigned_t<Int>>(i) >= 0x100u) {
					throw ByteTrunc{"int to byte conversion loses data"};
				}
			}
			return std::byte{static_cast<unsigned char>(i)};
		}

	//	AsHex function
	//
	//	Converts a std::byte into a 2-digit hexadecimal string.
	//
	//	Template args:
	//		Capitalize: return "AB" rather than "ab"? (defaults to false)
	//
	//	Function args:
	//		value: a std::byte
	//
	//	Returns:
	//		a std::string containing 2 hexadecimal digits
	//
	//	Note:
	//		Thanks to the small-string optimization, most standard library
	//		implementations should not allocate any dynamic memory for a
	//		string as short as this, though this cannot be guaranteed.
	//
	template<bool Capitalize=false>
		auto AsHex(std::byte value) -> std::string {
			auto hexDigit = [](unsigned i) constexpr -> char {
				if constexpr(Capitalize) {
					return "0123456789ABCDEF"[i & 0xF];
				}
				else {
					return "0123456789abcdef"[i & 0xF];
				}
			};
			unsigned i = std::to_integer<unsigned>(value);
			return {hexDigit(i >> 4), hexDigit(i)};
		}
	
	//-------------------------------------------------------------------------
	//
	//	Byte-Like Type Handling
	//
	//	Byte-like types include std::byte and any integral types with a size of
	//	1, such as char.
	//
	//-------------------------------------------------------------------------

	template<typename T>
		concept ByteLike = std::same_as<T, std::byte> or
			std::integral<T> and (sizeof(T) == 1);
	
	//	ByteLikeIt concept:
	//		This matches any iterator whose value_type is byte-like.
	template<typename T>
		concept ByteLikeIt =
			ByteLike<typename std::iterator_traits<T>::value_type>;
	
	//	ByteLikeContigIt concept:
	//		A byte-like contiguous iterator type.
	template<typename T>
		concept ByteLikeContigIt =
			ByteLikeIt<T> and std::contiguous_iterator<T>;

	//	ConvByteLike function:
	//		Converts a value from one byte-like type to another.
	template<ByteLike To, ByteLike From> constexpr
		auto ConvByteLike(From v) noexcept -> To {
			using std::byte, std::same_as;
			if constexpr(same_as<From,byte>) {
				if constexpr(same_as<To,byte>) {
					return v;
				}
				else {
					return std::to_integer<To>(v);
				}
			}
			else
				if constexpr(same_as<To,byte>) {
					return static_cast<unsigned char>(v);
				}
				else {
					return static_cast<To>(v);
				}
		}

	//-------------------------------------------------------------------------
	//
	//	Byte Order Checking
	//
	//-------------------------------------------------------------------------

	//	LittleEndian function
	//
	//	Returns:
	//		bool: compiler target uses little-endian byte order?
	//
	constexpr auto LittleEndian() noexcept -> bool
		{ return std::endian::native == std::endian::little; }

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
	
	//	BytePack function
	//
	//	Packs a numeric value into an array of bytes.
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
	//		T (inferred from function arg "v"): type of the numeric value
	//
	//	Function args:
	//		v: a numeric value
	//
	//	Returns:
	//		a std::array of Byte elements containing the packed "v" value
	//
	template<
		std::size_t Size=0, ByteLike Byte=TStreamByte,
		std::integral T
		>
		constexpr auto BytePack(T v) noexcept
			-> std::array<Byte, kBytePackSize<T, Size>>
		{
			constexpr std::size_t N = kBytePackSize<T, Size>;
			if constexpr(N == 1U) {
				return std::array<Byte,1U>{
					ConvByteLike<Byte>(static_cast<unsigned char>(v))
					};
			}
			else {
				using U8 = std::underlying_type_t<std::byte>;
				std::array<Byte, N> arr;
				auto end = arr.rend();
				for(auto it = arr.rbegin(); it != end; ++it, v >>= 8) {
					*it = ConvByteLike<Byte>(static_cast<U8>(v & 0xffU));
				}
				return arr;
			}
		}
	template<
		std::size_t Size=0, ByteLike Byte=TStreamByte,
		std::floating_point T
		>
		BINON_BIT_CAST_CONSTEXPR auto BytePack(T v) noexcept
			-> std::array<Byte, kBytePackSize<T, Size>>
		{
			using std::array;
			constexpr std::size_t N = kBytePackSize<T, Size>;
			static_assert(N == 4 or N == 8,
				"BinON expects binary32 or binary64 floating-point types");
			using U8 = std::underlying_type_t<std::byte>;
			BINON_BIT_CAST_CONSTEXPR auto pack =
				[](auto v) BINON_BIT_CAST_CONSTEXPR noexcept
			{
				using Arr = array<Byte, sizeof v>;
			 #if BINON_BIT_CAST
				if constexpr(LittleEndian()) {
					auto arr = std::bit_cast<Arr>(v);
					std::reverse(arr.begin(), arr.end());
					return arr;
				}
				else {
					return std::bit_cast<Arr>(v);
				}
			 #else
				Arr arr;
				std::memcpy(arr.data(), &v, sizeof v);
				if constexpr(LittleEndian()) {
					std::reverse(arr.begin(), arr.end());
				}
				return arr;
			 #endif
			};
			if constexpr(N == 4) {
				return pack(static_cast<types::TFloat32>(v));
			}
			else {
				return pack(static_cast<types::TFloat64>(v));
			}
		}
	template<ByteLike Byte=TStreamByte> constexpr
		auto BytePack(std::byte v) noexcept -> std::array<std::byte, 1U> {
			return std::array<Byte,1U>{ConvByteLike<Byte>(v)};
		}
	
	//	ByteUnpack
	//
	//	Unpack a numeric value packed into an array earlier by BytePack().
	//
	//	Template args:
	//		T: type of the numeric value
	//		Size (inferred from "arr" function arg): number of bytes to unpack
	//		Byte (inferred from "arr" function arg): array element type
	//			This should a byte-like type.
	//
	//	Function args:
	//		arr: array of type Byte and size Size containing packed bytes
	//
	//	Returns:
	//		value of type T unpacked from array
	//
	template<std::unsigned_integral T, ByteLike Byte, std::size_t Size>
		constexpr auto ByteUnpack(const std::array<Byte, Size>& arr) noexcept
			-> T
		{
			if constexpr(Size == 1U) {
				return ConvByteLike<unsigned char>(arr[0]);
			}
			else {
				T v{};
				for(auto b: arr) {
					v <<= 8;
					v |= ConvByteLike<unsigned char>(b);
				}
				return v;
			}
		}
	template<
		std::signed_integral T, std::size_t Size=sizeof(T),
		ByteLike Byte=TStreamByte
		>
		constexpr auto ByteUnpack(const std::array<Byte, Size>& arr) noexcept
			-> T
		{
			using U = std::make_unsigned_t<T>;
			auto u = ByteUnpack<U>(arr);
			if constexpr(Size >= sizeof(T)) {
				return static_cast<T>(u);
			}
			else {
				U m = U{1} << (Size * 8U - 1U);
				T v = static_cast<T>(u & (m - 1U));
				if(u & m) {
					v -= static_cast<T>(m);
				}
				return v;
			}
		}
	template<
		std::floating_point T, std::size_t Size=sizeof(T),
		ByteLike Byte=TStreamByte
		>
		BINON_BIT_CAST_CONSTEXPR
			auto ByteUnpack(std::array<Byte, Size> arr) noexcept -> T
		{
			using std::memcpy;
			static_assert(Size == 4 or Size == 8,
				"BinON expects binary32 or binary64 floating-point types");
			if constexpr(LittleEndian()) {
				std::reverse(arr.begin(), arr.end());
			}
			if constexpr(Size == 4) {
				types::TFloat32 v;
			 #if BINON_BIT_CAST
				v = bit_cast<decltype(v)>(arr);
			 #else
				memcpy(&v, arr.data(), Size);
			 #endif
				return static_cast<T>(v);
			}
			else {
				types::TFloat64 v;
			 #if BINON_BIT_CAST
				v = bit_cast<decltype(v)>(arr);
			 #else
				memcpy(&v, arr.data(), Size);
			 #endif
				return static_cast<T>(v);
			}
		}
	template<std::same_as<std::byte> T, ByteLike Byte, std::size_t Size>
		constexpr auto ByteUnpack(const std::array<Byte, Size>& arr) noexcept
			-> std::byte
		{
			return ConvByteLike<std::byte>(arr[0]);
		}

	//	WriteAsBytes function
	//
	//	Calls BytePack to pack a numeric value into bytes before writing the
	//	bytes to a binary output stream.
	//
	//	Template args:
	//		Size: see PackBytes
	//		T (inferred from "v" function arg): type the numeric value
	//
	//	Function args:
	//		stream: a binary output stream
	//		v: the value to stream out
	//		requireIO: throw exception if write fails? (defaults to true)
	//			Setting this false does not necessarily mean exceptions are
	//			disabled. The stream will be accessed as-is in that case.
	//
	template<std::size_t Size=0, typename T>
		void WriteAsBytes(TOStream& stream, T v, bool requireIO=true) {
			auto arr = BytePack<Size>(v);
			RequireIO rio(stream, requireIO);
			stream.write(arr.data(), arr.size());
		}
		
	//	ReadAsBytes function
	//
	//	Calls ByteUnpack to unpack a numeric value from a byte sequence read
	//	from a binary input stream.
	//
	//	Template args:
	//		T: type of numeric value
	//		Size: number of bytes to read and unpack (default to sizeof(T))
	//
	//	Function args:
	//		stream: a binary input stream
	//		requireIO: throw exception if read fails? (defaults to true)
	//			Setting this false does not necessarily mean exceptions are
	//			disabled. The stream will be accessed as-is in that case.
	//
	//			If exceptions ARE disabled, the return value may be corrupt.
	//			You should check the iostate bits of the stream to determine
	//			whether the value can be trusted?
	//
	//	Returns:
	//		numeric value of type T read from stream and unpacked
	//
	template<typename T, std::size_t Size=sizeof(T)>
		auto ReadAsBytes(TIStream& stream, bool requireIO=true) -> T {
			std::array<TStreamByte, Size> arr;
			RequireIO rio(stream, requireIO);
			stream.read(arr.data(), arr.size());
			return ByteUnpack<T>(arr);
		}

}

#endif
