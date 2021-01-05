#ifndef BINON_LITERALS_HPP
#define BINON_LITERALS_HPP

#include "byteutil.hpp"
#include "floattypes.hpp"

#include <stdexcept>
#include <cstdint>
#include <limits>

namespace binon {
	inline namespace literals {
		class BadLiteral: public std::out_of_range {
		public:
			using std::out_of_range::out_of_range;
		};

		namespace details {
			constexpr void AssertRange(bool inRange, const char* errMsg) {
					if(!inRange) {
						throw BadLiteral{errMsg};
					}
				}
		}

		//	std::byte's implementation does not seem to include a format for
		//	byte literals. So with this user-defined literal, you can now write
		//	0x42_byte instead of std::byte{0x42}.
		constexpr auto operator ""_byte(unsigned long long i) {
				details::AssertRange(
					i <= std::numeric_limits<std::uint8_t>::max(),
					"_byte literal out of range"
				);
				return ToByte(i);
			}

		//	_f32 and _f64 give you TFloat32 and TFloat64 literals,
		//	respectively. They are range-checked but not precision-checked.
		//	This means you cannot make an inf or nan float literal, nor say
		//	a 1.0e50_f32 that's too big for a 32-bit float.
		constexpr auto operator ""_f32(long double x) {
				details::AssertRange(
					x >= std::numeric_limits<types::TFloat32>::lowest() &&
					x <= std::numeric_limits<types::TFloat32>::max(),
					"_f32 literal out of range"
					);
				return static_cast<types::TFloat32>(x);
			}
		constexpr auto operator ""_f64(long double x) {
				details::AssertRange(
					x >= std::numeric_limits<types::TFloat64>::lowest() &&
					x <= std::numeric_limits<types::TFloat64>::max(),
					"_f64 literal out of range"
					);
				return static_cast<types::TFloat64>(x);
			}

		//	<cstdint> does not define any literals for integer types.
		//	Here we define only those that have a definite size like
		//	std::int32_t (_i32) and not those like std::int_fast32_t which
		//	are of less interest to BinON.
		constexpr auto operator ""_i8(unsigned long long i) {
				details::AssertRange(
					i <= std::numeric_limits<std::uint8_t>::max(),
					"_i8 literal out of range"
				);
				return static_cast<std::int8_t>(i);
			}
		constexpr auto operator ""_u8(unsigned long long i) {
				details::AssertRange(
					i <= std::numeric_limits<std::uint8_t>::max(),
					"_u8 literal out of range"
					);
				return static_cast<std::uint8_t>(i);
			}
		constexpr auto operator ""_i16(unsigned long long i) {
				details::AssertRange(
					i <= std::numeric_limits<std::uint16_t>::max(),
					"_i16 literal out of range"
					);
				return static_cast<std::int16_t>(i);
			}
		constexpr auto operator ""_u16(unsigned long long i) {
				details::AssertRange(
					i <= std::numeric_limits<std::uint16_t>::max(),
					"_u16 literal out of range"
					);
				return static_cast<std::uint16_t>(i);
			}
		constexpr auto operator ""_i32(unsigned long long i) {
				details::AssertRange(
					i <= std::numeric_limits<std::uint32_t>::max(),
					"_i32 literal out of range"
					);
				return static_cast<std::int32_t>(i);
			}
		constexpr auto operator ""_u32(unsigned long long i) {
				details::AssertRange(
					i <= std::numeric_limits<std::uint32_t>::max(),
					"_u32 literal out of range"
					);
				return static_cast<std::uint32_t>(i);
			}
		constexpr auto operator ""_i64(unsigned long long i) {
				details::AssertRange(
					i <= std::numeric_limits<std::uint64_t>::max(),
					"_i64 literal out of range"
					);
				return static_cast<std::int64_t>(i);
			}
		constexpr auto operator ""_u64(unsigned long long i) {
				details::AssertRange(
					i <= std::numeric_limits<std::uint64_t>::max(),
					"_u64 literal out of range"
					);
				return static_cast<std::uint64_t>(i);
			}
	}
}

#endif
