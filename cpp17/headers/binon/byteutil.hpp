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

	//	ByteLike concept:
	//		Matches a std::byte or any 1-byte integral type.
	template<typename T>
		concept ByteLike = std::same_as<T, std::byte> or
			std::integral<T> and (sizeof(T) == 1U);

	//	ByteOrInt concept:
	//		Matches a std::byte or any integral type.
	template<typename T>
		concept ByteOrInt = std::same_as<T, std::byte> or std::integral<T>;

	//	NonbyteInt concept:
	//		Matches all integral types that are NOT ByteLike. In other words,
	//		they must be at least 2 bytes in size.
	template<typename T>
		concept NonbyteInt = std::integral<T> and (sizeof(T) > 1U);

	//	NonbyteSInt and NonbyteUInt concepts:
	//		These are like NonbyteInt except they narrow the integral type to
	//		either signed or unsigned, respectively.
	template<typename T>
		concept NonbyteSInt = std::signed_integral<T> and (sizeof(T) > 1U);
	template<typename T>
		concept NonbyteUInt = std::unsigned_integral<T> and (sizeof(T) > 1U);

	//	ByteLikeIt concept:
	//		This matches any iterator whose value_type is byte-like.
	template<typename T>
		concept ByteLikeIt =
			ByteLike<typename std::iterator_traits<T>::value_type>;

	//	ByteLikeCast function:
	//		Casts a value to a byte-like type. (Narrowing may occur.)
	template<ByteLike To, typename From>
		constexpr auto ByteLikeCast(From v) noexcept -> To {
			using std::byte, std::same_as;
			if constexpr(same_as<From, byte>) {
				if constexpr(same_as<To, byte>) {
					return v;
				}
				else {
					return std::to_integer<To>(v);
				}
			}
			else
				if constexpr(same_as<To, byte>) {
					return byte{static_cast<unsigned char>(v)};
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
	//-------------------------------------------------------------------------

	template<typename T>
		concept BytePackable = std::same_as<T, std::byte> or
			std::integral<T> or std::floating_point<T>;

	template<typename T, std::size_t N>
		constexpr std::size_t kBytePackSize = N == 0 ? sizeof(T) : N;

	template<ByteLike B, BytePackable T, std::size_t N>
		using ByteArray = std::array<B, kBytePackSize<T, N>>;

	namespace details {
		template<std::input_iterator It>
			constexpr auto AdvanceIt(It it, std::size_t n) noexcept -> It {
				if constexpr(std::random_access_iterator<It>) {
					it += n;
				}
				else { for(; n > 0; --n, ++it) {} }
				return it;
			}

		template<std::size_t N=0, ByteLikeIt It, BytePackable T>
			constexpr auto BytePackPad(It it, T v) noexcept -> It {
				using B = typename std::iterator_traits<It>::value_type;
				constexpr auto n = kBytePackSize<T, N>;
				if constexpr(n > sizeof(T)) {
					B pad = ByteLikeCast<B>('\x00');
					if constexpr(std::signed_integral<T>) {
						if(v < 0) {
							pad = ByteLikeCast<B>('\xff');
						}
					}
					it = std::fill_n(it, n - sizeof(T), pad);
				}
				return it;
			}

		template<ByteLike B, std::floating_point T> BINON_BIT_CAST_CONSTEXPR
			auto BytePackFP(T v) noexcept -> std::array<B, sizeof(T)> {
				using Arr = std::array<B, sizeof(T)>;
				alignas(T) Arr arr;
			 #if BINON_BIT_CAST
				arr = std::bit_cast<Arr>(v);
			 #else
			 	std::memcpy(arr.data(), &v, sizeof(T));
			 #endif
			 	if constexpr(LittleEndian()) {
					std::reverse(arr.begin(), arr.end());
				}
				return arr;
			}

		template<std::floating_point T, ByteLikeIt It>
			BINON_BIT_CAST_CONSTEXPR auto ByteUnpackFP(It it) noexcept
				-> std::pair<T, It>
			{
				using B = typename std::iterator_traits<It>::value_type;
				using Arr = std::array<B, sizeof(T)>;
				constexpr bool randAcc = std::random_access_iterator<It>;
				alignas(T) Arr arr;
				if constexpr(LittleEndian()) {
					if constexpr(randAcc) {
						std::reverse_copy(it, it + sizeof(T), arr.begin());
					}
					else {
						std::copy_n(it, sizeof(T), arr.begin());
						std::reverse(arr.begin(), arr.end());
					}
				}
				else { std::copy_n(it, sizeof(T), arr.begin()); }
			 #if BINON_BIT_CAST
			 	T v = std::bit_cast<T>(arr);
			 #else
			 	T v;
			 	std::memcpy(&v, arr.data(), sizeof(T));
			 #endif
				return {v, AdvanceIt(it, sizeof(T))};
			}
	}

	template<std::size_t N=0, ByteLikeIt It, ByteLike T>
		constexpr auto BytePack(It it, T v) noexcept -> It {
			using B = typename std::iterator_traits<It>::value_type;
			it = details::BytePackPad<N, It, T>(it, v);
			return *it = ByteLikeCast<B>(v), ++it;
		}
	template<std::size_t N=0, ByteLikeIt It, NonbyteInt T>
		constexpr auto BytePack(It it, T v) noexcept -> It {
			using B = typename std::iterator_traits<It>::value_type;
			it = details::BytePackPad<N, It, T>(it, v);
			constexpr auto n = std::min(kBytePackSize<T, N>, sizeof(T));
			auto i = (static_cast<int>(n) - 1) * 8;
			do { *it++ = ByteLikeCast<B>(v >> i); }
			while((i -= 8) >= 0);
			return it;
		}
	template<std::size_t N=0, ByteLike B=TStreamByte, ByteOrInt T>
		constexpr auto BytePack(T v) noexcept -> ByteArray<B, T, N> {
			ByteArray<B, T, N> arr;
			return BytePack<N>(arr.begin(), v), arr;
		}
	template<std::size_t N=0, ByteLike B=TStreamByte, std::floating_point T>
		BINON_BIT_CAST_CONSTEXPR auto BytePack(T v) noexcept
			-> ByteArray<B, T, N>
		{
			using namespace details;
			using namespace types;
			constexpr auto n = kBytePackSize<T, N>;
			static_assert(n == 4 or n == 8,
				"BinON floating-point values must be encoded in 32 or 64 bits"
				);
			if constexpr(n == 4) {
				return BytePackFP<B>(static_cast<TFloat32>(v));
			}
			else {
				return BytePackFP<B>(static_cast<TFloat64>(v));
			}
		}
	template<std::size_t N=0, ByteLikeIt It, std::floating_point T>
		BINON_BIT_CAST_CONSTEXPR auto BytePack(It it, T v) noexcept -> It {
			using B = typename std::iterator_traits<It>::value_type;
			alignas(T) auto arr = BytePack<N, B, T>(v);
			return std::copy(arr.begin(), arr.end(), it);
		}
	template<bool ReqIO=kRequireIO, std::size_t N=0, BytePackable T>
		void BytePack(TOStream& stream, T v) {
			auto pack = [&] {
				alignas(T) auto arr = BytePack<N, TStreamByte, T>(v);
				stream.write(arr.data(), arr.size());
			};
			if constexpr(ReqIO) {
				RequireIO reqIO{stream};
				pack();
			}
			else { pack(); }
		}
	template<std::size_t N=0, BytePackable T>
		void BytePack(TOStream& stream, T v, bool reqIO) {
			if(reqIO) {
				BytePack<kRequireIO, N, T>(stream, v);
			}
			else {
				BytePack<kSkipRequireIO, N, T>(stream, v);
			}
		}

	template<ByteLike T, std::size_t N=sizeof(T), ByteLikeIt It>
		constexpr auto ByteUnpack(It it) noexcept -> std::pair<T, It> {
			static_assert(N > 0U);
			if constexpr(N > 1U) {
				it = details::AdvanceIt(it, N - 1U);
			}
			T v = ByteLikeCast<T>(*it);
			return {v, ++it};
		}
	template<NonbyteUInt T, std::size_t N=sizeof(T), ByteLikeIt It>
		constexpr auto ByteUnpack(It it) noexcept -> std::pair<T, It> {
			static_assert(N > 0U);
			T v{};
			for(auto i = N; i > 0U; --i, ++it) {
				v <<= 8;
				v |= ByteLikeCast<unsigned char>(*it);
			}
			return {v, it};
		}
	template<NonbyteSInt T, std::size_t N=sizeof(T), ByteLikeIt It>
		constexpr auto ByteUnpack(It it) noexcept -> std::pair<T, It> {
			using U = std::make_unsigned_t<T>;
			auto [u, it2] = ByteUnpack<U, N, It>(it);
			auto v = static_cast<T>(u);
			if constexpr(sizeof(T) > N) {
				constexpr auto m = T{1} << (N * 8 - 1);
				if(v & m) {
					v -= m << 1;
				}
			}
			return {v, it2};
		}
	template<std::floating_point T, std::size_t N=sizeof(T), ByteLikeIt It>
		BINON_BIT_CAST_CONSTEXPR auto ByteUnpack(It it) noexcept
			-> std::pair<T, It>
		{
			using namespace details;
			using namespace types;
			static_assert(N == 4 or N == 8,
				"BinON floating-point values must be encoded in 32 or 64 bits"
				);
			if constexpr(N == 4) {
				auto [v, it2] = ByteUnpackFP<TFloat32>(it);
				return {static_cast<T>(v), it2};
			}
			else {
				auto [v, it2] = ByteUnpackFP<TFloat64>(it);
				return {static_cast<T>(v), it2};
			}
		}
	template<BytePackable T, bool ReqIO=kRequireIO, std::size_t N=sizeof(T)>
		auto ByteUnpack(TIStream& stream) -> T {
			auto unpack = [&] {
				alignas(T) std::array<TStreamByte, N> arr;
				stream.read(arr.data(), arr.size());
				auto [v, it] = ByteUnpack<T, N>(arr.begin());
				return v;
			};
			if constexpr(ReqIO) {
				RequireIO reqIO{stream};
				return unpack();
			}
			else { return unpack(); }
		}
	template<BytePackable T, std::size_t N=sizeof(T)>
		auto ByteUnpack(TIStream& stream, bool reqIO) -> T {
			return reqIO ?
				ByteUnpack<T, kRequireIO, N>(stream) :
				ByteUnpack<T, kSkipRequireIO, N>(stream);
		}

}

#endif
