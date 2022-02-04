#ifndef BINON_HASHUTIL_HPP
#define BINON_HASHUTIL_HPP

#include "byteutil.hpp" // for the CHAR_BIT assertion
#include "typeutil.hpp"

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
	template<typename... Vs> constexpr
		auto HashCombine(std::size_t v, Vs... vs) noexcept
			-> std::enable_if_t<kArgsOfType<std::size_t, Vs...>, std::size_t>
		{
			using std::size_t;
			constexpr auto pred = [](size_t a, size_t b) {
				return details::HashCombine2(a, b);
			};
			return (
				BinPred(v, pred) + ... +
				BinPred(static_cast<size_t>(vs), pred)
			);
		}

	//	This is a high-level function that calls the std::hash functor on one
	//	or more values and returns a combined hash value for all of them.
	template<typename V> constexpr
		auto Hash(const V& v) -> std::size_t {
			return std::hash<V>{}(v);
		}
	template<typename V, typename... Vs> constexpr
		auto Hash(const V& v, const Vs&... vs) -> std::size_t {
			return details::HashCombine2(Hash(vs...), std::hash<V>{}(v));
		}

}

#endif
