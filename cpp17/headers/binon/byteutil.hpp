#ifndef BINON_BYTEUTIL_HPP
#define BINON_BYTEUTIL_HPP

#include "errors.hpp"
#include "ioutil.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>

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
	//		string as short as this, but this cannot be guaranteed.
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
	//	Byte Packing Functions
	//
	//-------------------------------------------------------------------------

	//	Number concept
	//
	//	This matches any integral or floating-point type. It is used in
	//	conjunction with the PackNumber() and UnpackNumber() functions defined
	//	further down.
	//
	template<typename T>
		concept Number = std::is_arithmetic_v<T>;

	//	ByteIt concept
	//
	//	This matches any iterator whose value_type is a single byte type like
	//	char, std::byte, etc. Again, this is used by PackNumber() and
	//	UnpackNumber().
	//
	template<typename T>
		concept ByteIt =
			(sizeof(typename std::iterator_traits<T>::value_type) == 1U);

	//	LittleEndian function
	//
	//	Returns:
	//		bool: compiler target uses little-endian byte order?
	//
	constexpr auto LittleEndian() noexcept -> bool
		{ return std::endian::native == std::endian::little; }

	//	PackNumber function
	//
	//	This function takes an integer or floating-point number and packs it
	//	into a binary sequence following a big-endian byte order convention.
	//	It is strongly recommended that you call it with types that have a
	//	well-defined size, such as std::int32_t.
	//
	//	Args:
	//		num: the number to pack
	//		it: an output iterator whose value_type is std::byte
	//
#if BINON_BIT_CAST
	constexpr void PackNumber(Number auto num, ByteIt auto it) noexcept {
			using T = typename std::iterator_traits<It>::value_type;
			alignas(decltype(num)) std::array<T, sizeof num> tmp;
			tmp = std::bit_cast<decltype(tmp)>(num);
			if constexpr(LittleEndian()) {
				std::reverse_copy(tmp.begin(), tmp.end(), it);
			}
			else std::copy(tmp.begin(), tmp.end(), it);
		}
#else
	void PackNumber(Number auto num, ByteIt auto it) noexcept {
			using T = typename std::iterator_traits<decltype(it)>::value_type;
			alignas(decltype(num)) std::array<T, sizeof num> tmp;
			std::memcpy(tmp.data(), &num, sizeof num);
			if constexpr(LittleEndian()) {
				std::reverse_copy(tmp.begin(), tmp.end(), it);
			}
			else std::copy(tmp.begin(), tmp.end(), it);
		}
#endif

	//	UnpackNumber function
	//
	//	This function unpacks a number from a sequence of bytes after having
	//	been packed by PackNumber().
	//
	//	Template args:
	//		Num: the numeric return type
	//		It: inferred from the functin arg: it
	//
	//	Function args:
	//		it: an input iterator whose value_type is std::byte
	//
	//	Returns:
	//		a number read from the input iterator's bytes
	//
#if BINON_BIT_CAST
	template<Number Num, ByteIt It> constexpr
		auto UnpackNumber(It it) noexcept -> Num {
			using T = typename std::iterator_traits<It>::value_type;
			alignas(Num) std::array<T, sizeof(Num)> tmp;
			if constexpr(LittleEndian()) {
				if constexpr(std::random_access_iterator<It>) {
					std::reverse_copy(it, it + sizeof(Num), tmp.begin());
				}
				else {
					std::copy_n(it, sizeof(Num), tmp.begin());
					std::reverse(tmp.begin(), tmp.end());
				}
			}
			else {
				std::copy_n(it, sizeof(Num), tmp.begin());
			}
			return std::bit_cast<Num>(tmp);
		}
#else
	template<Number Num, ByteIt It>
  		auto UnpackNumber(It it) noexcept -> Num {
			using T = typename std::iterator_traits<It>::value_type;
			alignas(Num) std::array<T, sizeof(Num)> tmp;
			if constexpr(LittleEndian()) {
				if constexpr(std::random_access_iterator<It>) {
					std::reverse_copy(it, it + sizeof(Num), tmp.begin());
				}
				else {
					std::copy_n(it, sizeof(Num), tmp.begin());
					std::reverse(tmp.begin(), tmp.end());
				}
			}
			else {
				std::copy_n(it, sizeof(Num), tmp.begin());
			}
			Num num;
			std::memcpy(&num, tmp.data(), sizeof num);
			return num;
		}
#endif

		void WriteNumber(
			TOStream& stream, Number auto num, bool requireIO = true)
		{
			std::array<TStreamByte, sizeof num> tmp;
			PackNumber(num, tmp.begin());
			RequireIO rio(stream, requireIO);
			stream.write(tmp.data(), tmp.size());
		}

		template<Number Num>
			auto ReadNumber(TIStream& stream, bool requireIO = true) -> Num
		{
			std::array<TStreamByte, sizeof(Num)> tmp;
			RequireIO rio(stream, requireIO);
			stream.read(tmp.data(), tmp.szie());
			return UnpackNum<Num>(tmp.begin());
		}

}

#endif
