#ifndef BINON_OPTUTIL_HPP
#define BINON_OPTUTIL_HPP

#include <functional>
#include <optional>
#include <utility>

namespace binon {

	//	OptRef class template
	//
	//	std::optional<std::reference_wrapper<T>> can be a very useful data type,
	//	but is a bit annoying in that you often to do a kind of
	//	double-derefencing to reach your T value, particularly when implicit
	//	type conversion is not quite up to the job of unravelling it. This class
	//	lets you get straight to the goods.
	//
	//	For example, if you wrote
	//
	//		auto optInt = OptRef{42};
	//
	//	*optInt return an int& of your value 42, as opposed to a
	//	std::reference_wrapper<int>.
	//
	//	The class is not entirely complete at this point in terms of
	//	implementing all the std::optional APIs. You can call as_optional() if
	//	you need to do something using the internal
	//	std::optional<std::reference_wrapper<T>> data structure.
	template<typename T>
		class OptRef: std::optional<std::reference_wrapper<T>> {
		 public:
			using TOptRef = std::optional<std::reference_wrapper<T>>;
			using value_type = T;

			using std::optional<std::reference_wrapper<T>>::optional;
			using std::optional<std::reference_wrapper<T>>::operator bool;
			using std::optional<std::reference_wrapper<T>>::has_value;
			using std::optional<std::reference_wrapper<T>>::swap;
			using std::optional<std::reference_wrapper<T>>::reset;

			BINON_IF_CPP20(constexpr)
				auto operator-> () const& noexcept -> const T* {
					return &TOptRef::operator*().get();
				}
			BINON_IF_CPP20(constexpr)
				auto operator-> () & noexcept -> T* {
					return &TOptRef::operator*().get();
				}
			BINON_IF_CPP20(constexpr)
				auto operator* () const& noexcept -> const T& {
					return TOptRef::operator*().get();
				}
			BINON_IF_CPP20(constexpr)
				auto operator* () & noexcept -> T& {
					return TOptRef::operator*().get();
				}
			BINON_IF_CPP20(constexpr)
				auto operator* () && noexcept -> T {
					return std::move(TOptRef::operator*().get());
				}
			BINON_IF_CPP20(constexpr)
				auto value() const& -> const T& {
					return TOptRef::value().get();
				}
			BINON_IF_CPP20(constexpr)
				auto value() & -> T& {
					return TOptRef::value().get();
				}
			BINON_IF_CPP20(constexpr)
				auto value() && -> T {
					return std::move(TOptRef::value().get());
				}

			//	value_or() works a bit differently from std::optional in this
			//	class unless you are using move semantics. In the non-move case,
			//	the default value must be an L-value reference of type T in
			//	order for the return value to also be an L-value reference of
			//	type T. (std::optional simply returns a value--not a
			//	reference--but that sort of defeats the purpose of working with
			//	an optional reference wrapper.)
			BINON_IF_CPP20(constexpr)
				auto value_or(const T& defVal) const& -> const T& {
					return *this ? **this : defVal;
				}
			BINON_IF_CPP20(constexpr)
				auto value_or(T& defVal) & -> T& {
					return *this ? **this : defVal;
				}
			template<typename U> BINON_IF_CPP20(constexpr)
				auto value_or(U&& defVal) && -> T {
					return *this ? *std::move(*this)
						: static_cast<T>(std::forward<U>(defVal));
				}

			constexpr auto as_optional() const& -> const TOptRef& {
					return *this;
				}
			constexpr auto as_optional() & -> TOptRef& {
					return *this;
				}
			constexpr auto as_optional() && -> TOptRef {
					return std::move(*this);
				}
		};

	//	MakeOpt function template
	//
	//	Depending on the bool you pass to this function, it will return either a
	//	value obtained from a callback or std::nullopt.
	//
	//	Template Args:
	//		T (type, required): value type that is optionally returned
	//		GetT (type, inferred)
	//		GetTArgs (types, inferred)
	//
	//	Args:
	//		hasValue (bool): condition indicating whether value is available
	//		getValue (functor): T fn(...) or std::optional<T> fn(...)
	//			Called if hasValue is true to obtain the value.
	//		args... (GetTArgs, optional): extra args passed to getValue
	//
	//	Returns:
	//		std::optional<T>: an optional value
	//			Derived from either getValue's return value or std::nullopt
	//			depending on whether hasValue is true.
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
}

#endif
