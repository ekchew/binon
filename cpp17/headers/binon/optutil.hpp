#ifndef BINON_OPTUTIL_HPP
#define BINON_OPTUTIL_HPP

#include <optional>
#include <type_traits>
#include <utility>

namespace binon {

	/**
	MakeOpt function template

	Depending on the bool you pass to this function, it will return either a
	value obtained from a callback or std::nullopt.

	Template Args:
		T (type, required): value type that is optionally returned
		GetT (type, inferred)
		GetTArgs (types, inferred)

	Args:
		hasValue (bool): condition indicating whether value is available
		getValue (functor): T fn(...) or std::optional<T> fn(...)
			Called if hasValue is true to obtain the value.
		args... (GetTArgs, optional): extra args passed to getValue

	Returns:
		std::optional<T>: an optional value
			Derived from either getValue's return value or std::nullopt
			depending on whether hasValue is true.
	**/
	template<typename T, typename GetT, typename... GetTArgs> constexpr
		auto MakeOpt(bool hasValue, GetT getValue, GetTArgs&&... args)
			-> std::optional<T>
		{
			return hasValue
				? std::optional<T>{
					getValue(std::forward<GetTArgs>(args)...)}
				: std::nullopt;
		}
}

#endif
