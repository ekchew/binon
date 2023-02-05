#ifndef BINON_TYPEUTIL_HPP
#define BINON_TYPEUTIL_HPP

#include "macros.hpp"

#include <concepts>
#include <type_traits>
#include <utility>
#include <variant>

namespace binon {

	namespace details {
		template<typename T, typename Variant>
			struct IsVariantMember;
		template<typename T, typename... Types>
			struct IsVariantMember<T, std::variant<Types...>>:
				std::disjunction<std::is_same<T, Types>...> {};
	}

	//---- VariantMember concept ----------------------------------------------
	//
	//	This restricts types to only those belonging to a specified variant.
	//
	template<typename T, typename Variant>
		concept VariantMember = details::IsVariantMember<T,Variant>::value;

	//---- CustomFold class ---------------------------------------------------
	//
	//	Given a pack of parameters of type T (or at least implicitly
	//	convertible to a type T), this class lets you apply a custom functor to
	//	fold the parameters into a single T value. It does so by overloading
	//	the + operator to call your functor.
	//
	//	For example, say you had a pack of integers 3, 4, and 5, and you wanted
	//	to fold them by doubling each number before adding the next one. In
	//	other words, you wanted to calculate:
	//
	//		(3 * 2 + 4) * 2 + 5
	//
	//	You could write:
	//
	//		constexpr int calculate(std::convertible_to<int> auto... ints) {
	//			auto fn = [](int a, int b) constexpr { return a * 2 + b; };
	//			return (... + MakeCustomFold<int>(ints, fn));
	//		}
	//
	//	and later call:
	//
	//		std::cout << calculate(3, 4, 5) << '\n';
	//
	//	Output:
	//
	//		25
	
	//	Your custom fold function should take 2 args of type T and return a
	//	T value.
	template<typename Fn, typename T>
		concept CustomFoldFn = requires(Fn fn, T a, T b) {
			{ Fn(a, b) } -> std::convertible_to<T>;
		};
	
	template<typename T, CustomFoldFn<T> Fn> struct CustomFold {
	 	using function_type = Fn;
		using value_type = T;

		Fn fn;
		T value;

		//	If you are certain all the parameters share the exact same type,
		//	you can use this constructor and infer the type from the v arg.
		//	But calling the MakeCustomFold() factory function should be safer.
		constexpr CustomFold(T&& v, Fn&& fn):
			v{std::forward<T>(v)}, fn{std::forward<Fn>(fn)} {}

		//	The fold expression evaluates to a CustomFold object. You can get
		//	the final value out of this by either accessing the value field
		//	directly or relying on implicit conversion to the type T.
		constexpr operator const T&() const& noexcept { return this->value; }
		constexpr operator T&() & noexcept { return this->value; }
		constexpr operator T() && { return std::move(this->value); }
	};

	template<typename T, CustomFoldFn<T> Fn> constexpr
		auto operator + (CustomFold<T,Fn>&& lhs, CustomFold<T,Fn>&& rhs) {
			using std::move;
			return CustomFold<T,Fn>{
				rhs.fn(move(lhs.value), move(rhs.value)),
				move(lhs.fn)
				};
		}

	//	MakeCustomFold factory function
	//
	//	This function returns a CustomFold<T> where T is specified as an
	//	explicit template arg. The value you actually pass in as v need not
	//	match T's type exactly as long as it can implicitly convert to it.
	//
	//	Template args:
	//		T: the data type for the fold
	//
	//	Function args:
	//		v: the value for this instance of CustomFold
	//		fn: your custom folding functor
	//			Note that for best performance, you want to keep this lean.
	//			Don't capture a huge amount of state, since it will be copied
	//			nearly 2N times, since that's how many CustomFold instances get
	//			constructed during the folding process.

	template<typename T, std::convertible_to<T> U, CustomFoldFn<T> Fn>
		constexpr auto MakeCustomFold(U&& v, Fn&& fn) {
			using std::forward;
			return CustomFold<T,Fn>{forward<U>(v), forward<Fn>(fn)};
		}

}

#endif
