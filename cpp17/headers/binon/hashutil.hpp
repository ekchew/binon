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

 #if BINON_CONCEPTS
	template<typename T>
		concept Hashable = requires(T a) {
			{ std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
		};
 #endif

	//	This is a high-level function that calls the std::hash functor on one or
	//	more hashable objects and returns a combined hash value for all of them.
 #if BINON_CONCEPTS
	template<Hashable... Vs> constexpr
 #else
	template<typename... Vs> constexpr
 #endif
		auto HashCombineObjs(const Vs&... vs) -> std::size_t {
			return HashCombine(std::hash<Vs>{}(vs)...);
		}

	//	CummutativeHash helps you combine hash values in a commutative manner.
	//	That way, you should get the same final result regardless of the order
	//	in which you combine them.
	//
	//	Such an algorithm is needed if you want to hash any of the
	//	std::unordered_... containers. The algorithm used here is based on what
	//	Python uses for frozenset hashing. Since said algorithm has a specific
	//	seed value and finalization step, it has been implemented here as a
	//	class rather than a simple function.
	struct CommutativeHash {
		std::size_t mHash = 1927868237UL;
	 public:

		//	The extend() method combines a new hash value with the one that is
		//	currently stored.
		void extend(std::size_t hashVal);

		//	You can call a CommutativeHash instance as a functor with any
		//	hashable value as its sole argument. It will then hash said value
		//	and call extend on it.
	#if BINON_CONCEPTS
		template<Hashable T>
	#else
		template<typename T>
	#endif
			void operator() (const T& v) { extend(std::hash<T>{}(v)); }

		//	The get() method finalizes the combined hash and returns it to you.
		//	(You can still extend the hash afterwards and call get() again.)
		//	CommutativeHash also implements a conversion operator to size_t that
		//	calls get implicitly.
		auto get() const -> std::size_t;
		operator std::size_t() const { return get(); }
	};

	//	This is nothing more than a random number generated at program launch
	//	you can apply to your hash values with a simple bitwise-xor (^)
	//	operation. It will ensure that every time your program runs, it will use
	//	different hash values. This can protect you against certain types of
	//	DDOS attacks.
	//
	//	Note that the std::hash specialization for BinONObj already applies the
	//	salt. (It rotates the salt by 1 bit left before the xor though in case
	//	you forget and xor it anyway.)
	//
	//	(A few technical notes. While gHashSalt may not have been declared
	//	const, you should not be going around changing it all the time. Once
	//	possible scenario for changing it would be if your project includes
	//	subprocesses and you want them to all share the same salt. Another would
	//	be if your program runs a long time and you want to periodically refresh
	//	the salt. In this case though, it is your responsibility to make sure
	//	nothing is still relying on the old salt before you do so. Also note
	//	that due to the generally read-only nature of gHashSalt, it has NOT been
	//	declared atomic.)
	extern std::size_t gHashSalt;
}

#endif
