#ifndef BINON_OPTUTIL_HPP
#define BINON_OPTUTIL_HPP

#include <optional>
#include <type_traits>
#include <utility>

namespace binon {

	/**
	MakeOpt function template

	MakeOpt resembles std::make_optional except that it adds an extra hasValue
	argument at the beginning.
	
	For example:
	
		auto optX = binon::MakeOpt(x > 0, x);
	
	is equivalent to:
	
		std::optional<std::decay_t<decltype(x)>> optX;
		if(x > 0) {
			optX = std::make_optional(x);
		}
		else {
			optX = std::nullopt;
		}
	
	Template Args:
		typename T: value type
		Remaining args: see std::make_optional

	Args:
		hasValue (bool):
			If true, return std::make_optional called on any remaining args.
			If false, return std::nullopt as the appropriate type.
		Remaining args: see std::make_optional
	
	Returns:
		std::optional<std::decay_t<T>>
	**/
	template<typename T>
	constexpr auto MakeOpt(bool hasValue, const T& value)
			-> std::optional<std::decay_t<T>>
		{
			if(hasValue) {
				return std::make_optional(value);
			}
			else {
				return std::nullopt;
			}
		}
	template<typename T>
	constexpr auto MakeOpt(bool hasValue, T&& value)
			-> std::optional<std::decay_t<T>>
		{
			if(hasValue) {
				return std::make_optional(std::move(value));
			}
			else {
				return std::nullopt;
			}
		}
	template<typename T, typename... Args>
	constexpr auto MakeOpt(bool hasValue, Args&&... args)
			-> std::optional<T>
		{
			if(hasValue) {
				return std::make_optional<T>(
					std::forward<Args>(args)...);
			}
			else {
				return std::nullopt;
			}
		}
	template<typename T, typename U, typename... Args>
	constexpr auto MakeOpt(
			bool hasValue, std::initializer_list<U> il, Args&&... args
			)
			-> std::optional<T>
		{
			if(hasValue) {
				return std::make_optional<T>(
					il, std::forward<Args>(args)...);
			}
			else {
				return std::nullopt;
			}
		}
	/**
	MakeOptByFn function template
	
	This variation on MakeOpt takes a functor that returns a value rather than
	an actual value. It works best in situations in which you do not want to
	calculate a value if hasValue is false. The functor in that case will simply
	not get called.
	
	Template Args:
		typename T: value type
		typename Fn: inferred from fn function arg
		typename... Args: inferred from args function args

	Args:
		hasValue (bool):
			If true, return std::make_optional called on your functor's return
			value. If false, return std::nullopt as the appropriate type.
		fn (functor): functor taking the form: T functor(Args&&...);
		args... (Args...): any extra args for the functor
	
	Returns:
		std::optional<T>
	**/
	template<typename T, typename Fn, typename... Args>
	constexpr auto MakeOptByFn(bool hasValue, Fn fn, Args&&... args)
			-> std::optional<T>
		{
			if(hasValue) {
				return std::make_optional<T>(fn(std::forward<Args>(args)...));
			}
			else {
				return std::nullopt;
			}
		}
}

#endif
