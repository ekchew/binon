#ifndef BINON_REFUTIL_HPP
#define BINON_REFUTIL_HPP

#include <functional>

/**
refutil module

Contains type definitions related to std::reference_wrapper.
**/

namespace binon {

	namespace details {
		template<typename T> struct Ref {
			using Base = T;
		};
		template<typename T> struct Ref<std::reference_wrapper<T>> {
			using Base = T;
		};
		template<typename T> struct Ref<const std::reference_wrapper<T>> {
			using Base = const T;
		};
	}

	/**
	Type Definitions:
		TRefBase<T>:
			This is equivalent to T in all cases except when T is a
			std::reference_wrapper<U>. In that case TRefBase<T> will be U.
			(Note: If the reference_wrapper is const, U will also be const.)
		TUnwrappedRef<T>: equivalent to TRefBase<T>&
		TUnwrappedRRef<T>: equivalent to TRefBase<T>&&
	**/
	template<typename T>
		using TRefBase = typename details::Ref<T>::Base;
	template<typename T>
		using TUnwrappedRef = typename details::Ref<T>::Base&;
	template<typename T>
		using TUnwrappedRRef = typename details::Ref<T>::Base&&;
}

#endif
