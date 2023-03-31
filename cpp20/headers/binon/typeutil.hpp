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
	//	This class template allows you to customize the behaviour of fold
	//	expressions (introduced in C++17). You supply it with a value of type T
	//	and a functor that takes two Ts and combines them into one. The class
	//	then overloads the + operator to invoke your functor.
	//
	//	For example, say you wanted to build up an integer by doubling it
	//	before adding the next value from the parameter pack. If your pack
	//	consisted of the numbers 3, 4, and 5, you would want to calculate:
	//
	//		(3 * 2 + 4) * 2 + 5
	//
	//	You could write:
	//
	//		#include <iostream>
	//
	//		template<typename... Ints> constexpr int foo(Ints... ints) noexcept
	//		{
	//			auto foldFn = [](int a, int b) constexpr noexcept {
	//				return a * 2 + b;
	//			};
	//			return (... + MakeCustomFold<int>(ints, foldFn));
	//		}
	//
	//		int main() {
	//			std::cout << foo(3, 4, 5) << '\n';
	//		}
	//
	//	Output:
	//
	//		25
	//
	//	A few points to note:
	//	  -	All parameters in the parameter pack would ideally be of the exact
	//		same data type. If that is true, you can let the constructor deduce
	//		the type T. In other words, for the well-behaved case above, we
	//		could have written:
	//
	//			return (... + CustomFold{ints, foldFn});
	//
	//	  -	Failing that, the parameters must at least be implicitly
	//		convertible to a type T. The MakeCustomFold() function lets you
	//		specify this type explicitly through a template arg, which is
	//		generally safer.
	//
	//	  -	CustomFold takes the functor arg by L-value reference. That means
	//		you cannot define the lambda within the fold expression (since it
	//		would be an R-value in that case). You would not want to define it
	//		there anyway since it would create multiple functor objects where
	//		you would only need one.
	//
	//	  -	If you declare your functor noexcept, this will get propagated up
	//		through the APIs, for what it's worth. All APIs are also marked
	//		constexpr, so marking your lambda constexpr where applicable is
	//		also encouraged. (FYI: constexpr lambdas were introduced in C++17.)
	//
	//	  -	The above example would not handle the case where ints is an empty
	//		parameter pack (i.e. you called foo() without args). This is a
	//		general issue with fold expressions and nothing specific to
	//		CustomFold. It is just something to be aware of and work around if
	//		need be.

 	template<typename Fn, typename T>
		concept CustomFoldFn = requires(Fn fn, T a, T b) {
			{ fn(a, b) } -> std::convertible_to<T>;
		};
 
	template<typename T, CustomFoldFn<T> Fn>
		struct CustomFold
	{
		//	Template arg type definitions.
		using value_type = T;
		using functor_type = Fn;

		//	The two data members of this class, which can be aggregate-
		//	initialized (using curly {}) with template arg deduction, though
		//	calling the MakeCustomFold() factory function is preferable.
		//
		//	The functor should accept 2 T args and return a T. Note that the
		//	args get passed to your functor by R-value reference, so move
		//	construction applies is you accept them by value.
		T value;
		Fn& functor;

		//	While CustomFold is an open struct that makes all of its members
		//	public, there are implicit conversion/assignment operators defined
		//	for the T value as a convenience.
		constexpr operator const T& () const& noexcept { return this->value; }
		constexpr operator T&() & noexcept { return this->value; }
		constexpr operator T() &&
			noexcept(std::is_nothrow_move_constructible_v<T>) {
				return std::move(this->value);
			}
		constexpr auto& operator = (const T& v)
			noexcept(std::is_nothrow_copy_assignable_v<T>) {
				return this->value = v, *this;
			}
		constexpr auto& operator = (T&& v)
			noexcept(std::is_nothrow_move_assignable_v<T>) {
				return this->value = std::move(v), *this;
			}

		constexpr auto operator + (CustomFold&& rhs) &&
				noexcept(
					std::is_nothrow_move_constructible_v<T> and
					noexcept(
						std::declval<Fn>()(
							std::declval<T>(), std::declval<T>()
							)
						)
					)
			{
				return CustomFold{
					this->functor(std::move(this->value), std::move(rhs.value)),
					this->functor
					};
			}
	};

	//	MakeCustomFold() is a factory function that returns a CustomFold
	//	instance. It expects you to specify an explicit type "T" in a template
	//	arg which may or may not match the "value" function arg's type (though
	//	the latter must be implicitly convertible to T).
	//
	//	Template args:
	//		T: a type to which all parameters can be implicitly converted
	//		U: inferred from the "value" function arg
	//		Fn: inferred from the "functor" function arg
	//
	//	Function args:
	//		value: a value implicitly convertible to type T
	//		functor: a functor taking 2 T args and return a T
	//			Note that the args get passed in by R-value reference.
	//
	//	Returns:
	//		a CustomFold<T,Fn> instance built around "value" and "functor"

	template<typename T, std::convertible_to<T> U, CustomFoldFn<T> Fn>
		constexpr auto MakeCustomFold(U&& value, Fn& functor)
			noexcept(noexcept(T{std::forward<U>(value)}))
		{
			return CustomFold<T,Fn>{std::forward<U>(value), functor};
		}

}

#endif
