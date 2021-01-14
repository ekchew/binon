#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "macros.hpp"
#include "optutil.hpp"

#include <cstddef>
#include <functional>
#include <iterator>

namespace binon {

	/**
	Empty struct

	This is a blank struct with no members of any kind. The MakeGenerator
	template function defaults to this type for the its Data template argument,
	and Generator template classes handle the Empty type in a special way.
	**/
	struct Empty {};

	/**
	Generator class template
	
	A Generator is an input iterable class built around a functor your supply.
	You instantiate one by calling the MakeGenerator template function.
	
	The functor's job is to keep supplying the Generator's iterator with its
	next value until all of the values are exhausted. It is often called the
	"nextFn" for this reason.
	
	It can take one of two forms:
	
		std::optional<T> nextFn();
		std::optional<T> nextFn(Data& data);
	
	Here, T is the value type you want the iterator to produce. You return the
	values wrapped in std::optional until you run out, at which point you can
	return std::nullopt instead to signal the iteration loop to exit.
	
	The first form in which nextFn takes no arguments is the default. You get
	the second form by supplying a custom data type in MakeGenerator's first
	template argument. The reference will be to the Generator's mData member,
	and you can do whatever you want with it.

	Example 1: Print integers from 1 to 5
		Functors are typically implemented using lambda functions. This example
		demonstrates one way that you can have a variable that changes every
		time your lambda gets called: in this case, an integer counter called
		"i". The idea is to make the counter a mutable captured value.

		Source:
			auto gen = binon::MakeGenerator(
				[i = 0]() mutable {
					return ++i, binon::MakeOpt(i <= 5, i);
				});
			for(auto i: gen) {
				std::cout << i << '\n';
			}

		Output:
			1
			2
			3
			4
			5

	Example 2: Same thing, but use Generator data to hold counter
		This time, we tell MakeGenerator to store an int (first template arg) we
		initialize to 0 (second function arg). Then an int& is supplied to the
		functor and we can increment it as in the first example.

		Source:
			auto gen = binon::MakeGenerator<int>(
				[](int& i) {
					return ++i, binon::MakeOpt(i <= 5, i);
				}, 0);
			for(auto i: gen) {
				std::cout << i << '\n';
			}

			//	"*gen" is equivalent to "gen.mData". (If your data type were a
			//	class/struct, you could also use the "->" operator to access its
			//	individual members.)
			std::cout << "final i: " << *gen << '\n';

		Output:
			1
			2
			3
			4
			5
			final i: 6

	Template Args:
		typename Data: type of mData member
		typename NextFn: type of mNextFn
		typename... DataArgs: args used to initialize mData member

	Type definitions:
		value_type: type emitted by this generator
		iterator: input iterator of value_type
		const_iterator: input iterator of const value_type

		TNextFn: NextFn
		TData: Data
		TOptVal: std::optional<value_type>
			This is the type returned by TNextFn.
		IterBase: parent class template of both iterator and const_iterator
			Implements the shared functionality of the subclasses (which is
			actually most of the functionality).
		iterator:
			Returned by begin() and end() if the Generator instance is variable.
			
			Dereferencing the iterator may throw an exception in debug mode if
			no value is available. (See std::optional::value() for the nature of
			the exception.) In release mode, this check is suppressed.

			Though iterator, unlike const_iterator, allows you to modify the
			derefenced value, there may not be much point in doing so since it
			is a temporary value returned by mNextFn that will get overwritten
			the next time it is called (via the ++ operator). iterator's *
			operator does, however, support move semantics.
			
			You could, for example, write:
				
				std::string s = *std::move(myStringIter);
			
			to move a string out of its temporary home.
		const_iterator:
			Returned by begin() and end() if the Generator instance is constant,
			and also by cbegin() and cend() regardless. See the second paragraph
			under iterator regarding exceptions. It also applies to
			const_iterator.
	Data Members:
		mNextFn (NextFn): functor returning next generator value
			This functor should take the form:

				std::optional<T> nextFn(); // if Data is Empty
				std::optional<T> nextFn(Data& data); // if Data is anything else

			where T is the data type your generator emits.
	**/
	namespace details {
		//	GenNextFn is a struct for handling differences between the data and
		//	no data versions of Generator below.
		template<typename Data, typename NextFn, typename Enable=void>
		struct GenNextFn {
			using TOptVal = std::invoke_result_t<NextFn,Data&>;
			template<typename Gen>
				static auto Call(Gen& gen) { return gen.mNextFn(gen.mData); }
		};
		template<typename Data, typename NextFn>
		struct GenNextFn<Data, NextFn,
			std::enable_if_t<std::is_same_v<Data,Empty>>
			>
		{
			using TOptVal = std::invoke_result_t<NextFn>;
			template<typename Gen>
				static auto Call(Gen& gen) { return gen.mNextFn(); }
		};
	}
	template<typename Data, typename NextFn>
	struct Generator {
		using TOptVal = typename details::GenNextFn<Data,NextFn>::TOptVal;
		using TNextFn = NextFn;
		using TData = Data;
		using value_type = typename TOptVal::value_type;
		static_assert(
			std::is_same_v<TOptVal, std::optional<value_type>>,
			"binon::Generator NextFn must return a std::optional");

		//	Base class of both iterator and const_iterator. Never instantiated
		//	directly.
		template<typename ItCls>
		struct IterBase {
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
			const auto& operator * () const& { return this->value(); }
			const auto* operator -> () const { return &this->value(); }
			const auto& operator ++ () const
				{ return callNextFn(), asItCls(); }
			const auto& operator ++ (int) const
				{ ItCls copy{asItCls()}; return *++this, copy; }

		protected:
			mutable Generator* mPGen;
			mutable TOptVal mOptVal;

			auto& value() & {
				#if BINON_DEBUG
					return mOptVal.value();
				#else
					return *this->mOptVal;
				#endif
				}
			auto&& value() && {
				#if BINON_DEBUG
					return std::move(mOptVal).value();
				#else
					return *std::move(this->mOptVal);
				#endif
				}
			const auto& value() const& {
				#if BINON_DEBUG
					return mOptVal.value();
				#else
					return *this->mOptVal;
				#endif
				}
			void callNextFn() const
				{ mOptVal = details::GenNextFn<Data,NextFn>::Call(*mPGen); }

		private:
			const auto& asItCls() const noexcept
				{ return *static_cast<const ItCls*>(this); }
		};
		struct iterator: IterBase<iterator> {
			using reference = value_type&;
			using pointer = value_type*;
			using IterBase<iterator>::IterBase;
			using IterBase<iterator>::operator *;
			using IterBase<iterator>::operator ->;
			auto& operator * () & { return this->value(); }
			auto&& operator * () && { return std::move(*this).value(); }
			auto* operator -> () { return &this->value(); }
		};
		struct const_iterator: IterBase<const_iterator> {
			using reference = const value_type&;
			using pointer = const value_type*;
			using IterBase<const_iterator>::IterBase;
		};

		NextFn mNextFn;
		Data mData;

		template<typename... DataArgs>
			Generator(NextFn nextFn, DataArgs&&... dataArgs) noexcept:
				mNextFn{nextFn}, mData{std::forward<DataArgs>(dataArgs)...} {}

		/**
		operator * and -> overloads

		Dereferencing a Generator gives you access to its mData member.
		
		Note that * supports move semantics. For example, if your data type were
		a std::string, you could write:
			
			std::string s = *std::move(gen);
		
		to move the string out of gen.mData. (You would probably not want to do
		so until you are done iterating.)
		**/
		auto& operator * () & noexcept { return mData; }
		auto&& operator * () && noexcept { return std::move(mData); }
		const auto& operator * () const& noexcept { return mData; }
		auto operator -> () noexcept { return &mData; }
		const auto operator -> () const noexcept { return &mData; }

		/**
		begin, cbegin member functions

		mNextFn is called once by a newly instantiated iterator to load an
		initial value.

		Returns:
			Generator::iterator (or Generator::const_iterator)
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
			Generator::iterator (or Generator::const_iterator)
		**/
		auto end() noexcept { return iterator{*this}; }
		auto end() const noexcept { return const_iterator{*this}; }
		auto cend() const noexcept { return const_iterator{*this}; }
	};
	
	/**
	MakeGenerator function template
	
	Since it would be difficult to figure out the data type of your functor,
	it's easiest to call this helper function to instantiate a Generator.
	
	Template Args:
		typename Data (default-constructible, optional):
			This is the only template argument you may want to supply manually.
			You can set it to a data type of your choosing to give your functor
			a reference to an instance of type Data that persists between calls.
			The default binon::Empty struct disables this feature (i.e. your
			functor will receive no args).
		NextFn (functor): inferred from nextFn function arg
		DataArgs... (any types): inferred from nextFn dataArgs args
	
	Args:
		nextFn (NextFn): a functor that returns a std::optional
			See Generator class notes regarding how to write one.
		dataArgs... (DataArgs&&...):
			If you supplied a custom data type for the Data template argument,
			you can add any suitable arguments here to initialize an instance
			of Data. (This instance will be stored in the mData member of the
			returned Generator and passed to your nextFn by reference.)
	
	Returns:
		Generator<Data,NextFn,DataArgs...>
	**/
	template<typename Data=Empty, typename NextFn, typename... DataArgs>
		auto MakeGenerator(NextFn nextFn, DataArgs&&... dataArgs) {
			return Generator<Data,NextFn>{
				nextFn, std::forward<DataArgs>(dataArgs)...};
		}

	template<typename T, typename Data, typename Enable=void>
	class GenBase {
	protected:
		std::function<std::optional<T>(Data&)> mNextFn;
		Data mData;
	
	public:
		template<typename... DataArgs>
			GenBase(decltype(mNextFn) nextFn, DataArgs&&... args):
				mNextFn{nextFn}, mData{std::forward<DataArgs>(args)...} {}
		auto operator * () & noexcept -> Data& { return mData; }
		auto operator * () const& noexcept -> const Data& { return mData; }
		auto operator * () && noexcept -> Data&& { return std::move(mData); }
		auto operator -> () noexcept -> Data* { return &mData; }
		auto operator -> () const noexcept -> const Data* { return &mData; }
		auto callNextFn() -> std::optional<T> { return mNextFn(mData); }
	};
	template<typename T, typename Data>
	struct GenBase<
		T, Data, std::enable_if_t<std::is_same_v<Data, void>>
		>
	{
	protected:
		std::function<std::optional<T>()> mNextFn;
	
	public:
		GenBase(decltype(mNextFn) nextFn) noexcept: mNextFn{nextFn} {}
		auto callNextFn() -> std::optional<T> { return mNextFn(); }
	};
	template<typename T, typename Data=void>
	class Gen: GenBase<T,Data> {
	protected:
		using GenBase<T,Data>::mNextFn;
	
	public:
		using TValue = T;
		using TData = Data;
		using TNextFn = decltype(mNextFn);
		using value_type = TValue;
		
		struct iterator {
			using iterator_category = std::input_iterator_tag;
			using value_type = Gen::value_type;
			using difference_type = std::ptrdiff_t;
			using reference = value_type&;
			using pointer = value_type*;
			
			explicit iterator(const Gen& gen) noexcept:
				mPGen{const_cast<Gen*>(&gen)} {}
			explicit operator bool() const noexcept
				{ return mOptVal.has_value(); }
			auto operator == (const iterator& rhs) const noexcept
				{ return mOptVal == rhs.mOptVal; }
			auto operator != (const iterator& rhs) const noexcept
				{ return mOptVal != rhs.mOptVal; }
			auto operator * () & -> value_type&
				{ return BINON_IF_DBG_REL(mOptVal.value(), *mOptVal); }
			auto operator * () const& -> const value_type&
				{ return BINON_IF_DBG_REL(mOptVal.value(), *mOptVal); }
			auto operator * () && -> value_type&& {
					using std::move;
					return BINON_IF_DBG_REL(
						move(mOptVal).value(), *move(mOptVal));
				}
			auto operator -> () -> value_type*
				{ return &BINON_IF_DBG_REL(mOptVal.value(), *mOptVal); }
			auto operator -> () const -> const value_type*
				{ return &BINON_IF_DBG_REL(mOptVal.value(), *mOptVal); }
			auto operator ++ () -> iterator&
				{ return mOptVal = mPGen->callNextFn(), *this; }
			auto operator ++ () const -> const iterator&
				{ return mOptVal = mPGen->callNextFn(), *this; }
			auto operator ++ (int) const -> iterator {
					iterator copy{*this};
					return ++*this, copy;
				}
		
		private:
			Gen* mPGen;
			mutable std::optional<value_type> mOptVal;
		};
		
		using GenBase<T,Data>::GenBase;
		auto begin() const -> iterator { return ++iterator{*this}; }
		auto end() const -> iterator { return iterator{*this}; }
	};
}

#endif
