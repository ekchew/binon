#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "macros.hpp"
#include "optutil.hpp"

#include <cstddef>
#include <functional>
#include <iterator>

namespace binon {

	/**
	GeneratorBase class template

	This parent class of Generator has a specialization for dealing with the
	Data type void. See the Generator class and MakeGenerator function template
	docs for more info.
	**/
	template<typename T, typename Data, typename Functor>
	struct GeneratorBase {
		static_assert(
			std::is_same_v<
				std::invoke_result_t<Functor, Data&>, std::optional<T>
				>,
			"Generator functor must take the form: std::optional<T> fn(Data&)"
			);

		/**
		constructor

		GeneratorBase's constructor is exposed by Generator.

		Template Args:
			typename... DataArgs: inferred from dataArgs constructor arg

		Args:
			functor (Functor): your functor
			dataArgs... (DataArgs...): args to initialize a Data instance
				Clearly, these only make sense if your Data type is not void.
		**/
		template<typename... DataArgs>
			GeneratorBase(Functor functor, DataArgs&&... dataArgs):
				mFunctor{functor}, mData{std::forward<DataArgs>(dataArgs)...} {}

		/**
		operator overloads: *

		This operator gives you access to the internal Data instance (the same
		one that is passed to your functor by reference). It is only defined
		when your Data type is something other than void.

		There are 2 overloads: one gives you an L-value reference and the other
		an R-value reference. You can go *std::move(gen) to get the latter. (You
		probably wouldn't want to do so before you're done iterating though.)
		**/
		auto operator * () & noexcept -> Data& { return mData; }
		auto operator * () && noexcept -> Data&& { return std::move(mData); }

		/**
		operator overload: ->

		If your Data type is a class/struct, you can access members with ->. It
		is only defined when your Data type is something other than void.
		**/
		auto operator -> () noexcept -> Data* { return &mData; }

	protected:
		Functor mFunctor;
		Data mData;

		auto callFunctor() -> std::optional<T> { return mFunctor(mData); }
	};
	template<typename T, typename Functor>
	struct GeneratorBase<T, void, Functor> {
		static_assert(
			std::is_same_v<
				std::invoke_result_t<Functor>, std::optional<T>
				>,
			"Generator functor must take the form: std::optional<T> fn()"
			);

		GeneratorBase(Functor functor) noexcept: mFunctor{functor} {}

	protected:
		Functor mFunctor;

		auto callFunctor() -> std::optional<T> { return mFunctor(); }
	};

	/**
	Generator class template

	A Generator is an iterable class built around a functor you supply that
	returns a series of values over multiple calls. In its simplest form, the
	functor would look like this:

		std::optional<T> myFunctor();

	It would return values wrapped in std::optional until it runs out. At that
	point, it would return std::nullopt instead (or a default-constructed
	std::optional<T>{} if you prefer).

	The above form applies when the Data template argument is set to void (as it
	is by default when you call the MakeGenerator helper function). If you set
	the Data type to something other than void, Generator will store an internal
	instance of that type and pass it to your functor by reference.

		std::optional<T> myFunctor(Data& data); // for non-void Data type

	Example 1: Print integers from 1 to 5
		Functors are typically implemented as lambda functions. This example
		demonstrates one way that you could have a variable that changes every
		time your lambda gets called: in this case, the integer counter i. We
		capture i in mutable form into the lambda.

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
		This time, we tell MakeGenerator to store an int (1st template arg) we
		initialize to 0 (2nd function arg). Then an int& is supplied to the
		functor and we can increment it as in the first example.

		At the end, you can see that it is also possible to access your Data
		value externally by dereferencing your generator as though it were a
		pointer. (If your Data type were a struct{int i;}, you could also access
		the i data member with gen->i.)

		Source:
			auto gen = binon::MakeGenerator<int>(
				[](int& i) {
					return ++i, binon::MakeOpt(i <= 5, i);
				}, 0);
			for(auto i: gen) {
				std::cout << i << '\n';
			}
			std::cout << "final i: " << *gen << '\n';

		Output:
			1
			2
			3
			4
			5
			final i: 6

	Template Args:
		typename T: value type emitted by Generator
		typename Data: type of data passed to your functor by reference
		typename Functor: a functor you supply to generate values.
	**/
	template<typename T, typename Data, typename Functor>
	struct Generator: GeneratorBase<T,Data,Functor> {
		using TValue = T;
		using TData = Data;
		using TFunctor = Functor;
		using value_type = TValue;

		/**
		iterator class

		This is the input iterator class instantiated by Generator's begin() and
		end() methods. Its ++ operator calls on your functor to generate the
		next value, which you can then dereference with the * or -> operators.
		(The begin() method performs ++ for you once to get you started.)

		You can check if the std::optional your functor returned actually
		contains a value by testing the iterator, whose bool operator is
		overloaded for this purpose. Alternatively, you can compare it against
		the iterator returned by end().

		Should you dereference an iterator with no value, it may throw an
		exception, but likely only in debug mode. (See the
		std::optional<...>::value method docs for more info on the exception.)

		Note that Generator does not currently supply any const methods and
		therefore does not define a const_iterator class. Within the binon
		codebase, generators yield elements by value and there has been no need
		to protect the source from modification. The class may be expanded to
		include a const_iterator at some point should the need arise.

		The * operator does, however, support move semantics. Let's say your
		generator was yielding std::string objects. You could write

			std::string s = *std::move(myGen);

		to avoid an unnecessary copy.
		**/
		struct iterator {
			using iterator_category = std::input_iterator_tag;
			using value_type = Generator::value_type;
			using difference_type = std::ptrdiff_t;
			using reference = value_type&;
			using pointer = value_type*;

			explicit iterator(Generator& gen) noexcept:
				mPGen{&gen} {}
			explicit operator bool() noexcept
				{ return mOptVal.has_value(); }
			auto operator == (const iterator& rhs) noexcept
				{ return mOptVal == rhs.mOptVal; }
			auto operator != (const iterator& rhs) noexcept
				{ return mOptVal != rhs.mOptVal; }
			auto operator * () & -> reference
				{ return BINON_IF_DBG_REL(mOptVal.value(), *mOptVal); }
			auto operator * () && -> value_type&& {
					return BINON_IF_DBG_REL(
						std::move(mOptVal).value(), *std::move(mOptVal));
				}
			auto operator -> () -> pointer
				{ return &BINON_IF_DBG_REL(mOptVal.value(), *mOptVal); }
			auto operator ++ () -> iterator&
				{ return mOptVal = mPGen->callFunctor(), *this; }
			auto operator ++ (int) -> iterator {
					iterator copy{*this};
					return ++*this, copy;
				}

		private:
			Generator* mPGen;
			std::optional<value_type> mOptVal;
		};

		using GeneratorBase<T,Data,Functor>::GeneratorBase;

		auto begin() -> iterator { return ++iterator{*this}; }
		auto end() -> iterator { return iterator{*this}; }
	};

	/**
	MakeGenerator function template

	This helper function is essentially the same as instantiating a Generator by
	constructor except that it supplies a default value for the Data template
	argument: void

	Template Args:
		typename T (any type): the value type the generator produces
		typename Data (any type, optional): defaults to void
		typename Functor: inferred from functor function arg
		typename... DataArgs: inferred from dataArgs function args

	Args:
		functor (Functor): your functor
			It should take one of two forms:

				std::optional<T> functor();
				std::optional<T> functor(Data&);

			The second form applies if you supply a Data type other than void.
		dataArgs... (DataArgs...): any extra args to initialize a Data instance
			Clearly, these only make sense if your Data type is not void.

	Returns:
		Generator<T,Data,Functor>
	**/
	template<
		typename T, typename Data=void,
		typename Functor, typename... DataArgs
		>
	auto MakeGenerator(Functor functor, DataArgs&&... dataArgs)
		-> Generator<T,Data,Functor> {
			return Generator<T,Data,Functor>{
				functor, std::forward<DataArgs>(dataArgs)...};
		}

	template<typename ParentGen, typename ChildData>
	struct ChildGenData {
		using TParentGen = ParentGen;
		using TChildData = ChildData;
		using TParentIter = typename ParentGen::iterator;

		TParentGen parentGen;
		TParentIter parentIter;
		TChildData childData;

		template<typename... ChildArgs>
			ChildGenData(const TParentGen& parentGen, ChildArgs&&... args):
				parentGen{parentGen},
				childData{std::forward<ChildArgs>(args)...} {
					parentIter = parentGen.begin();
				}
		template<typename Functor>
			static auto WrapFunctor(Functor functor) {
				return [&](ChildGenData& cgd) {
					return functor(
						cgd.parentGen, cgd.parentIter, cgd.childData);
				};
			}
	};
	template<typename ParentGen>
	struct ChildGenData<ParentGen,void> {
		using TParentGen = ParentGen;
		using TChildData = void;
		using TParentIter = typename ParentGen::iterator;

		TParentGen parentGen;
		TParentIter parentIter;

		ChildGenData(const TParentGen& parentGen):
			parentGen{parentGen} {
				parentIter = parentGen.begin();
			}
		template<typename Functor>
			static auto WrapFunctor(Functor functor) {
				return [&](ChildGenData& cgd) {
					return functor(cgd.parentGen, cgd.parentIter);
				};
			}
	};

	template<
		typename ChildT, typename ChildData=void,
		typename ParentGen, typename Functor, typename... ChildArgs
		>
		auto ChainGenerators(
			const ParentGen& gen, Functor functor, ChildArgs&&... args)
		{
			using CGD = ChildGenData<ParentGen,ChildData>;
			return MakeGenerator<ChildT,CGD>(
				CGD::WrapFunctor(functor), gen,
				std::forward<ChildArgs>(args)...
			);
		}
}

#endif
