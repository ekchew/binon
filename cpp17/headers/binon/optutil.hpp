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
	constexpr auto MakeOpt(bool hasValue, Args&&... args) -> std::optional<T> {
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
			) -> std::optional<T>
		{
			if(hasValue) {
				return std::make_optional<T>(
					il, std::forward<Args>(args)...);
			}
			else {
				return std::nullopt;
			}
		}
}

#endif
