#ifndef BINON_HASHUTIL_HPP
#define BINON_HASHUTIL_HPP

#include "byteutil.hpp" // for the CHAR_BIT assertion
#include "typeutil.hpp"

#include <concepts>
#include <functional>

namespace binon {

	namespace details {

		//	This low-level function for combining 2 hash values is based on the
		//	boost implementation, but extends the magic number (apparently based
		//	on the golden ratio) if std::size_t is 64 bits.
		constexpr auto HashCombine2(std::size_t a, std::size_t b) noexcept
				-> std::size_t
			{
				constexpr decltype(a) kMagic =
					sizeof a == 8 ? 0x9e3779b97f4a7c15 : 0x9e3779b9;
				return a ^ (b + kMagic + (a << 6) + (a >> 2));
			}
	}

	//	Combines 2 or more hash values you generated using std::hash into a
	//	single value and returns it.
 #if BINON_CONCEPTS
	template<std::convertible_to<std::size_t>... Vs>
		auto HashCombine(std::size_t v, Vs... vs) noexcept -> std::size_t
 #else
	template<typename... Vs> constexpr
		auto HashCombine(std::size_t v, Vs... vs) noexcept
			-> std::enable_if_t<kArgsOfType<std::size_t, Vs...>, std::size_t>
 #endif
		{
			using std::size_t;
			return (
				CustomFold<details::HashCombine2,size_t>(v) + ... +
				CustomFold<details::HashCombine2,size_t>(vs)
			);
		}

	//	This is a high-level function that calls the std::hash functor on one
	//	or more values and returns a combined hash value for all of them.	/*
	template<typename... Vs> constexpr
		auto Hash(const Vs&... vs) -> std::size_t {
			return HashCombine(std::hash<Vs>{}(vs)...);
		}
}

#endif
