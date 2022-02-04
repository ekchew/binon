#ifndef BINON_TYPEUTIL_HPP
#define BINON_TYPEUTIL_HPP

#include "macros.hpp"
#include <type_traits>
#if BINON_CONCEPTS
	#include <concepts>
#endif

namespace binon {

	//	kArgsOfType helps check if all the arguments you pass to a variadic
	//	template function match the type you specify. Consider the example:
	//
	//		template<typename... Ints> auto SumInts(Ints... ints)
	//			-> std::enable_if_t<kArgsOfType<int,Ints...>, int>
	//		{
	//			return (0 + ... + ints);
	//		}
	//
	//	A SumInts() call will only compile if all its arguments are of type int
	//	(or intrinsically convertible to int).
	//
	//	You can get sort of a similar effect by using std::initializer_list, but
	//	this class has some limitations. For example, it does not support move
	//	semantics.
	//
	//	Note that in C++20, you don't really need kArgsOfType anymore since you
	//	can use the std::convertible_to concept like this:
	//
	//		template<std::convertible_to<int>... Ints>
	//			auto SumInts(Ints... ints) -> int
	//		{
	//			return (0 + ... + ints);
	//		}
	template<typename T, typename... Args>
		constexpr bool kArgsOfType =
			(std::is_convertible_v<Args,T> && ... && true);

	//	The BinPred struct template helps you set up a binary predicate that can
	//	be used in C++17 fold expressions by overloading the + operator.
	//	The HashCombine() function in hashutil.hpp demonstrates how it works.
	template<typename T, typename Pred>
		struct BinPred {
			using value_type = T;
			T value;
			Pred pred;
			constexpr BinPred(const T& v, const Pred& p):
				value{v}, pred{p} {}
			constexpr BinPred(T&& v, const Pred& p) noexcept:
				value{std::move(v)}, pred{p} {}
			constexpr BinPred(const Pred& p): pred{p} {}
			constexpr operator T&() & { return value; }
			constexpr operator const T&() const& { return value; }
			constexpr operator T() && { return std::move(value); }
			constexpr auto& operator = (const T& v) { return value = v, *this; }
			constexpr auto& operator = (T&& v) noexcept {
					return value = std::move(v), *this;
				}
			constexpr auto operator + (const BinPred& rhs) {
					return BinPred{pred(value, rhs.value), pred};
				}
			constexpr auto& operator += (const BinPred& rhs) {
					return value = pred(value, rhs.value), *this;
				}
		};

}

#endif
