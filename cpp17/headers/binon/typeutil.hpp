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
}

#endif
