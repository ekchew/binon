#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "macros.hpp"

#include <cstddef>
#include <iterator>
#include <optional>
#include <utility>
#include <type_traits>

namespace binon {

	/**
	MakeOpt function template
	
	MakeOpt resembles std::make_optional except that it adds an extra hasValue
	argument at the beginning.
	
	For example,
	
		if(hasValue) {
			return std::make_optional(x);
		}
		else {
			return std::nullopt;
		}
	
	can be shortened to:
		
		return MakeOpt(hasValue, x);
	
	Template Args:
		typename T: value type
		Remaining args: see std::make_optional
	
	Args:
		hasValue (bool):
			If true, std::make_optional is called on the remaining args.
			If false, return the default std::optional<T> with no value.
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
	
	/**
	Generator class template
	
	Given a functor that returns a new value each time it is called until the
	values are exhausted, Generator builds an iterator class around it so that
	you can iterate over it in range-based for loops and such.
	
	The functor should take no arguments but return a std::optional<T>, where T
	is your value type. You typically implement it using a lambda function.
	
	Example: Print integers from 1 to 5
		This example demonstrates how you can have a variable that changes every
		time your lambda function gets called: in this case, the integer counter
		i. The idea is to make it a mutable captured value.
		
		We capture i with an initial value of 0. Then we increment it before
		returning what MakeOpt decides to do with it. If it hasn't exceeded 5
		yet, it returns an optional (std::optional<int>) with the value i. But
		if i is greater than 5, it returns an optional with no value.
		
		Source:
			Generator gen{
				[i = 0]() mutable {
					return ++i, MakeOpt(i <= 5, i);
				}
			};
			for(auto i: gen) {
				std::cout << i << '\n';
			}
		
		Output:
			1
			2
			3
			4
			5
	
	Template Args:
		NextFn (functor): inferred from nextFn constructor arg
	
	Type definitions:
		value_type: type emitted by this generator
		iterator: input iterator of value_type
		const_iterator: input iterator of const value_type
		
		TNextFn: NextFn
		TOptVal: std::optional<value_type>
			This is the type returned by TNextFn.
		IterBase: base class template for both iterator and const_iterator
			It should never be instantiated directly.
	
	Data Members:
		mNextFn (NextFn): functor returning next generator value
			This functor should take the form:
				
				std::optional<T> FUNCTOR()
			
			where T is the data type your generator emits.
	**/
	template<typename NextFn>
	struct Generator {
		using TNextFn = NextFn;
		using TOptVal = std::invoke_result_t<NextFn>;
		using value_type = typename TOptVal::value_type;
		static_assert(
			std::is_same_v<TOptVal, std::optional<value_type>>,
			"binon::Generator NextFn must return a std::optional");
		
		//	Base class of both iterator and const_iterator. Never instantiated
		//	directly.
		template<typename Subcls> struct IterBase {
			using iterator_category = std::input_iterator_tag;
			using value_type = Generator::value_type;
			
			//	This is irrelvant to generators but may be needed to satisfy
			//	some iterator requirement?
			using difference_type = std::ptrdiff_t;
			
			explicit IterBase(Generator& gen) noexcept: mPGen{&gen} {}
			explicit operator bool() const noexcept
				{ return mOptVal.has_value(); }
			auto operator == (const IterBase& rhs) const noexcept
				{ return this->mOptVal == rhs.mOptVal; }
			auto operator != (const IterBase& rhs) const noexcept
				{ return this->mOptVal != rhs.mOptVal; }
			const auto& operator * () const { return this->value(); }
			const auto* operator -> () const { return &this->value(); }
			const auto& operator ++ () const
				{ return callNextFn(), asSubcls(); }
			const auto& operator ++ (int) const
				{ Subcls copy{asSubcls}; return *++this, copy; }
		
		protected:
			mutable Generator* mPGen;
			mutable TOptVal mOptVal;
			
			auto& value() const {
				#if BINON_DEBUG
					return mOptVal.value();
				#else
					*this->mOptVal;
				#endif
				}
			void callNextFn() const { mOptVal = mPGen->mNextFn(); }
		
		private:
			const auto& asSubcls() const noexcept
				{ return *static_cast<const Subcls*>(this); }
		};
		struct iterator: IterBase<iterator> {
			using reference = value_type&;
			using pointer = value_type*;
			using IterBase<iterator>::IterBase;
			auto& operator * () { return this->value(); }
			auto operator -> () { return &this->value(); }
		};
		
		struct const_iterator: IterBase<const_iterator> {
			using reference = const value_type&;
			using pointer = const value_type*;
			using IterBase<const_iterator>::IterBase;
		};
		
		NextFn mNextFn;
		
		/**
		Generator constructor - NextFn variant
		
		Args:
			nextFn (NextFn): functor initializing mNextFn
		**/
		Generator(NextFn nextFn) noexcept: mNextFn{nextFn} {}
		
		/**
		begin, cbegin member functions
		
		mNextFn is called once by a newly instantiated iterator to load an
		initial value.
		
		Returns:
			Generator::iterator or Generator::const_iterator
		**/
		auto begin() { return ++iterator{*this}; }
		auto begin() const { return ++const_iterator{*this}; }
		auto cbegin() const { return ++const_iterator{*this}; }
		
		/**
		end, cend member functions
		
		The default iterator with no value assigned to its internal
		std::optional<value_type>. Once mNextFn exhausts all of its values,
		it should return a blank optional that should match the cend value on
		comparison.
		
		Returns:
			Generator::iterator or Generator::const_iterator
		**/
		auto end() noexcept { return iterator{*this}; }
		auto end() const noexcept { return const_iterator{*this}; }
		auto cend() const noexcept { return const_iterator{*this}; }
	};

}

#endif
