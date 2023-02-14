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
	//	Byte Packing Functions
	//
	//	These are core to BinON's encoding of various numeric values into
	//	byte sequences following a big-endian ordering convention.
	//
	//-------------------------------------------------------------------------	
	
	//	BytePack function
	//
	//	Packs a numeric value into a sequence of bytes following a big-endian
	//	byte order convention.
	//
	//	For example, if you wanted to pack the integer 1000 into 2 bytes, it
	//	would encode as {'\x03', '\xe8'}.
	//
	//	Template args:
	//		Size: number of bytes to pack or 0
	//			The default 0 means the size of the input value "v" will
	//			represent the number of bytes. (If you go with the default,
	//			you should use a type with a well-defined size such as
	//			std::int32_t.)
	//		BL: a byte-like type (see above)
	//			Defaults to TStreamByte (usually char).
	//		T: inferring from the "v" function arg
	//
	//	Function args:
	//		v: a numeric value to pack
	//			This may be an integer or a floating-point value. In the latter
	//			case, the number of bytes to pack must evaluate to either 4 or
	//			8 (corresponding to the IEEE 754 binary32 and binary64 formats
	//			supported by BinON, respectively).
	//
	//	Returns:
	//		std::array<BL, N>, where N is the number of bytes packed
	//
	template<
		std::size_t Size=0, ByteLike BL=TStreamByte,
		std::integral T
		>
		constexpr auto BytePack(T v) noexcept
			-> std::array<BL, Size ? Size : sizeof(T)>
		{
			constexpr std::size_t N = Size ? Size : sizeof(T);
			if constexpr(N == 1U) {
				return std::array<BL,1U>{ConvByteLike<BL>(v)};
			}
			else {
				using U8 = std::underlying_type_t<std::byte>;
				std::array<BL, N> arr;
				auto end = arr.rend();
				for(auto it = arr.rbegin(); it != end; ++it, v >>= 8) {
					*it = ConvByteLike<BL>(static_cast<U8>(v & 0xffU));
				}
				return arr;
			}
		}
	template<
		std::size_t Size=0, ByteLike BL=TStreamByte,
		std::floating_point T
		>
		BINON_BIT_CAST_CONSTEXPR auto BytePack(T v) noexcept
			-> std::array<BL, Size ? Size : sizeof(T)>
		{
			using std::array;
			constexpr std::size_t N = Size ? Size : sizeof(T);
			static_assert(N == 4 or N == 8,
				"BinON expects binary32 or binary64 floating-point types");
			using U8 = std::underlying_type_t<std::byte>;
			BINON_BIT_CAST_CONSTEXPR auto pack =
				[](auto v) BINON_BIT_CAST_CONSTEXPR noexcept
			{
				using Arr = array<BL, sizeof v>;
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
	
	//	ByteUnpack function
	//
	//	Unpacks data packed earlier by BytePack().
	//
	//	Template args:
	//		T: the numeric type to unpack
	//			See notes regarding BytePack's "v" function arg.
	//		Size: number of bytes to unpack
	//			Default to sizeof(T). (If you go with the default, you should
	//			use a type with a well-defined size such as std::int32_t.)
	//		BL: a byte-like type (see above)
	//			Defaults to TStreamByte (usually char).
	//
	//	Function args:
	//		arr: L-value reference to a std::array<BL, Size>
	//			This should contain the byte data to unpack.
	//			WARNING:
	//				ByteUnpack may modify the data in arr. Specifically, it may
	//				reverse the byte order if the compiler target is a
	//				little-endian CPU. (In the current implementation, this
	//				only happens in the floating-point case.)
	//
	//	Returns:
	//		unpacked value of type T
	//
	template<
		std::unsigned_integral T, std::size_t Size=sizeof(T),
		ByteLike BL=TStreamByte
		>
		constexpr auto ByteUnpack(const std::array<BL, Size>& arr) noexcept
			-> T
		{
			T v{};
			for(auto b: arr) {
				v <<= 8;
				v |= ConvByteLike<BL>(static_cast<unsigned char>(b & 0xffU));
			}
			return v;
		}
	template<
		std::signed_integral T, std::size_t Size=sizeof(T),
		ByteLike BL=TStreamByte
		>
		constexpr auto ByteUnpack(const std::array<BL, Size>& arr) noexcept
			-> T
		{
			using U = std::make_unsigned_t<T>;
			auto u = ByteUnpack<U, Size, BL>(arr);
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
		ByteLike BL=TStreamByte
		>
		BINON_BIT_CAST_CONSTEXPR
			auto ByteUnpack(std::array<BL, Size>& arr) noexcept -> T
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

		//	WriteAsBytes function
		//
		//	This function calls BytePack to pack a numeric value before
		//	writing the byte sequence to a binary output stream.
		//
		//	Template args:
		//		Size: see BytePack function
		//		T: inferred from function "v" arg
		//
		//	Function args:
		//		stream: a binary output stream
		//		v: the value to write (see BytePack function)
		//		requireIO: throw exception on I/O error (defaults to true)
		//
		template<std::size_t Size=0, typename T>
			void ByteWrite(TOStream& stream, T v, bool requireIO=true) {
				auto arr = BytePack<Size>(v);
				RequireIO rio(stream, requireIO);
				stream.write(arr.data(), arr.size());
			}
		
		//	ReadAsBytes function
		//
		//	This function reads a byte sequence from a binary input stream and
		//	unpacks it into a numeric value by calling the ByteUnpack function.
		//
		//	Template args:
		//		T: the numeric type to unpack
		//			See notes regarding BytePack's "v" function arg.
		//		Size: number of bytes to unpack (defaults to sizeof(T))
		//			See ByteUnpack function.
		//	
		//	Function args:
		//		stream: a binary input stream
		//		requireIO: throw exception on I/O error (defaults to true)
		//			If false, the returned value may be corrupt. You should
		//			check the stream error flags before using it.
		//
		//	Returns:
		//		unpacked value of type T
		//
		template<typename T, std::size_t Size=sizeof(T)>
			auto ReadAsBytes(TIStream& stream, bool requireIO=true) -> T {
				std::array<TStreamByte, Size> arr;
				RequireIO rio(stream, requireIO);
				stream.read(arr.data(), arr.size());
				return ByteUnpack<T, Size>(arr);
			}

}

#endif
