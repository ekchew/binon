#ifndef BINON_TYPEUTIL_HPP
#define BINON_TYPEUTIL_HPP

#include "macros.hpp"
#include <type_traits>
#include <utility>
#include <variant>

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

	//	Some nifty code off the Internet that determines if a given type is
	//	among the possible members of a std::variant.
	template<typename T, typename Variant> struct IsVariantMember;
	template<typename T, typename... EveryT>
		struct IsVariantMember<T, std::variant<EveryT...>>:
			std::disjunction<std::is_same<T, EveryT>...>
		{
		};
	template<typename T, typename Variant>
		constexpr bool kIsVariantMember = IsVariantMember<T,Variant>::value;
 #if BINON_CONCEPTS
	template<typename Variant, typename T>
		concept VariantMember = kIsVariantMember<T,Variant>;
 #endif

	//	CustomFold is a struct template that allows you to apply custom folding
	//	behaviour in C++17 fold expressions. It stores a value of arbitrary type
	//	T (the 2nd template argument) which may be inferred from a value you
	//	pass into the constructor (though it is not a bad idea to declare it
	//	explicitly).
	//
	//	You must, however, always supply the 1st template argument. This is a
	//	function taking 2 arguments of type T and returning a T. What CustomFold
	//	does is overload the + operator to call your function instead.
	//
	//	For example, let's say at each stage, you wanted to double the current
	//	number and add the new one. If your arguments were 3, 4, and 5, you
	//	would be calculating (3 * 2 + 4) * 2 + 5 = 25. You could write this like
	//	so:
	//
	//		int fn(int a, int b) { return a * 2 + b; }
	//		template<typename... Ints> int calculate(Ints... ints) {
	//			return (... + CustomFold<fn,int>(ints));
	//		}
	//
	//	Note that C++17 does not allow you to use lambdas as template arguments.
	//	You need to pass in a regular function. But in C++20, you could write:
	//
	//		template<typename... Ints> int calculate(Ints... ints) {
	//			auto fn = [](int a, int b) { return a * 2 + b; };
	//			return (... + CustomFold<fn,int>(ints));
	//		}
#if BINON_CONCEPTS
	template<auto Fn, typename T>
		concept CustomFoldable = requires(T a, T b) {
			{ Fn(a, b) } -> std::convertible_to<T>;
		};
	template<auto Fn, typename T> requires CustomFoldable<Fn,T>
#else
	template<auto Fn, typename T>
#endif
		struct CustomFold {
			using value_type = T;
			T value = T();
			constexpr CustomFold(const T& v): value{v} {}
			constexpr CustomFold(T&& v) noexcept:
				value{std::move(v)} {}
			constexpr CustomFold() = default;
			constexpr operator T&() & { return value; }
			constexpr operator const T&() const& { return value; }
			constexpr operator T() && { return std::move(value); }
			constexpr auto& operator = (const T& v) {
					return value = v, *this;
				}
			constexpr auto& operator = (T&& v) noexcept {
					return value = std::move(v), *this;
				}
			constexpr auto operator + (const CustomFold& rhs) {
					return CustomFold{Fn(value, rhs.value)};
				}
			constexpr auto& operator += (const CustomFold& rhs) {
					return value = Fn(value, rhs.value), *this;
				}
		};

	//	kIsPair<T> evaluates true if T is a std::pair.
	template<typename T>
		struct IsPair: std::false_type {};
	template<typename A, typename B>
		struct IsPair<std::pair<A,B>>: std::true_type {};
	template<typename T>
		constexpr bool kIsPair = IsPair<T>::value;
 BINON_IF_CONCEPTS(
	template<typename T>
		concept Pair = kIsPair<T>;
 )
}

#endif
