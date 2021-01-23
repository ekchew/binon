#ifndef BINON_OPTUTIL_HPP
#define BINON_OPTUTIL_HPP

/**
optutil module

Contains utility functions related to std::optional.
**/

#include "refutil.hpp"

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
		auto MakeOpt(bool hasValue, GetT&& getValue, GetTArgs&&... args)
			-> std::optional<T>
		{
			using std::forward;
			return hasValue
				? std::optional<T>{
					forward<GetT>(getValue)(forward<GetTArgs>(args)...)}
				: std::nullopt;
		}

	/**
	EqualOpts function template

	In many cases, you can compare two std::optional with nothing more than the
	== operator, but there seem to be some edge cases involving
	std::reference_wrapper. For example, comparing
	optional<reference_wrapper<string>>>'s is problematic. Since the BinON
	library does sometimes do this sort of thing internally, it needed a
	function to work around this.

	Template Args:
		T (type, inferred)

	Args:
		optA (const std::optional<T>&): first optional value
		optB (const std::optional<T>&): second optional value

	Returns:
		bool: true if optA equals optB
	**/
	template<typename T> constexpr
		auto EqualOpts(
			const std::optional<T>& optA,
			const std::optional<T>& optB) noexcept
		{
			auto hasVal = optA.has_value();
			return hasVal == optB.has_value() && (!hasVal || (
				static_cast<TUnwrappedRef<const T>>(*optA) ==
				static_cast<TUnwrappedRef<const T>>(*optB)
				));
		}

	/**
	DerefOpt function template

	This function returns the value contained within a std::optional. Should
	there be no value available, it should throw std::bad_optional_access in
	debug mode only. (Otherwise, the behavior is undefined.)

	DerefOpt will also unwrap the value if it is a std::reference_wrapper,
	returning a normal T& reference instead.

	Template Args:
		T (type, inferred)

	Args:
		opt (std::optional<T>&): an optional value

	Returns:
		TUnwrappedRef<T>:
			This should be a reference with any std::reference_wrapper stripped
			off.
	**/
	template<typename T> constexpr
		auto DerefOpt(std::optional<T>& opt) -> TUnwrappedRef<T> {
			return BINON_IF_DBG_REL(opt.value(), *opt);
		}
	template<typename T> constexpr
		auto DerefOpt(std::optional<T>&& opt) -> TUnwrappedRRef<T> {
			return BINON_IF_DBG_REL(std::move(opt).value(), *std::move(opt));
		}
}

#endif
