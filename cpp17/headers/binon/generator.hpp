#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "macros.hpp"

#include <optional>
#include <type_traits>

namespace binon {

	template<typename NextFn>
	struct Generator {
		using optional_type = std::invoke_result_t<NextFn>;
		using value_type = typename optional_type::value_type;
		static_assert(
			std::is_same_v<optional_type, std::optional<value_type>>,
			"binon::Generator NextFn must return a std::optional");
		
		class const_iterator {
			const Generator* mPGen;
			optional_type mOptVal;
		public:
			const_iterator(const Generator& gen) noexcept: mPGen{&gen} {}
			explicit operator bool() const noexcept
				{ return mOptVal.has_value(); }
		#if BINON_DEBUG
			auto& operator * () const { return mOptVal.value(); }
			auto operator -> () const { return &mOptVal.value(); }
		#else
			auto& operator * () const noexcept { return *mValue; }
			auto operator -> () const noexcept { return &*mValue; }
		#endif
			auto& operator ++ () const
				{ return mOptVal = mPGen->mNextFn(), *this; }
			auto& operator ++ (int) const { 
				{ const_iterator copy{*this}; return *++this, copy; }
			auto operator == (const const_iterator& rhs) const noexcept
				{ return mOptVal == rhs.mOptVal; }
			auto operator != (const const_iterator& rhs) const noexcept
				{ return mOptVal != rhs.mOptVal; }
		};
		
		
		NextFn mNextFn;
		std::tuple<Args...> mArgs;

		Generator(NextFn nextFn) noexcept: mNextFn{nextFn} {}
		auto cbegin() const { return ++const_iterator{*this}; }
		auto cend() const noexcept { return const_iterator{*this}; }
	};

}

#endif
