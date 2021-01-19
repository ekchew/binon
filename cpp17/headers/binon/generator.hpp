#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "macros.hpp"
#include "optutil.hpp"

#include <cstddef>
#include <functional>
#include <iterator>

namespace binon {

	/**
	GenBase class template

	This parent class of Generator has a specialization for dealing with the
	Data type void. See the Generator class and MakeGen function template
	docs for more info.

	Template Args:
		T (type): see Generator
		Data (type): see Generator
		Functor (type): see Generator
	**/
	template<typename T, typename Data, typename Functor>
	struct GenBase {
		static_assert(
			std::is_same_v<
				std::invoke_result_t<Functor, Data&>, std::optional<T>
				>,
			"Generator functor must take the form: std::optional<T> fn(Data&)"
			);

		/**
		constructor

		GenBase's constructor is exposed by Generator.

		Template Args:
			DataArgs (types): inferred from dataArgs constructor args

		Args:
			functor (Functor): your functor
			dataArgs... (DataArgs...): args to initialize a Data instance
				Clearly, these only make sense if your Data type is not void.
		**/
		template<typename... DataArgs>
			GenBase(Functor functor, DataArgs&&... dataArgs):
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
	struct GenBase<T, void, Functor> {
		static_assert(
			std::is_same_v<
				std::invoke_result_t<Functor>, std::optional<T>
				>,
			"Generator functor must take the form: std::optional<T> fn()"
			);

		GenBase(Functor functor) noexcept: mFunctor{functor} {}

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
	is by default when you call the MakeGen helper function). If you set
	the Data type to something other than void, Generator will store an internal
	instance of that type and pass it to your functor by reference.

		std::optional<T> myFunctor(Data& data); // for non-void Data type

	Example 1: Print integers from 1 to 5
		Functors are typically implemented as lambda functions. This example
		demonstrates one way that you could have a variable that changes every
		time your lambda gets called: in this case, the integer counter i. We
		capture i in mutable form into the lambda.

		Source:
			auto gen = binon::MakeGen<int>(
				[i = 0]() mutable {
					return ++i, binon::MakeOpt<int>(i <= 5, [i] { return i; });
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
		This time, we tell MakeGen to store an int (1st template arg) we
		initialize to 0 (2nd function arg). Then an int& is supplied to the
		functor and we can increment it as in the first example.

		At the end, you can see that it is also possible to access your Data
		value externally by dereferencing your generator as though it were a
		pointer. (If your Data type were a struct{int i;}, you could also access
		the i data member with gen->i.)

		Source:
			auto gen = binon::MakeGen<int>(
				[](int& i) {
					return ++i, binon::MakeOpt<int>(i <= 5, [i] { return i; });
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
		T (type, required): value type emitted by Generator
		Data (type, required): optional extra data for functor
			Set to void if your functor needs no extra data. (See also
			MakeGen function template.)
		Functor (type, inferred):
			Takes one of 2 forms:
				std::optional<T> fn()
				std::optional<T> fn(Data&)
			The 1st form applies if the Data type is void.
	**/
	template<typename T, typename Data, typename Functor>
	struct Generator: GenBase<T,Data,Functor> {
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

			Generator* mPGen;
			std::optional<value_type> mOptVal;

			explicit iterator(Generator& gen) noexcept:
				mPGen{&gen} {}
			explicit operator bool() const noexcept
				{ return mOptVal.has_value(); }
			auto operator == (const iterator& rhs) const noexcept
				{ return mOptVal == rhs.mOptVal; }
			auto operator != (const iterator& rhs) const noexcept
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
		};

		using GenBase<T,Data,Functor>::GenBase;

		auto begin() -> iterator { return ++iterator{*this}; }
		auto end() -> iterator { return iterator{*this}; }
	};

	/**
	MakeGen function template

	This helper function is essentially the same as instantiating a Generator by
	constructor except that it supplies a default value for the Data template
	argument: void

	Template Args:
		T (type, required): the value type the generator produces
		Data (type, optional): defaults to void
		Functor (type, inferred): see Generator
		DataArgs (types, inferred)

	Args:
		functor (Functor): see Generator
		dataArgs... (DataArgs...): any extra args to initialize a Data instance
			Clearly, these only make sense if your Data type is not void.

	Returns:
		Generator<T,Data,Functor>
	**/
	template<
		typename T, typename Data=void,
		typename Functor, typename... DataArgs
		>
		auto MakeGen(Functor functor, DataArgs&&... dataArgs)
			-> Generator<T,Data,Functor> {
				return Generator<T,Data,Functor>{
					functor, std::forward<DataArgs>(dataArgs)...};
			}

	/**
	MakeIterGen function template

	This function wraps a Generator around a begin/end iterator pair.

	Template Args:
		BgnIt (type, inferred)
		EndIt (type, inferred)

	Args:
		bgnIt (BgnIt): begin iterator
		endIt (EndIt): end iterator

	Returns:
		Generator of BgnIt::value_type
	**/
	template<typename BgnIt, typename EndIt>
		auto MakeIterGen(BgnIt bgnIt, EndIt endIt) {
			using T = typename BgnIt::value_type;
			auto nextT = [](auto& it) {
				return *it++;
			};
			auto nextOptT = [bgnIt, endIt, nextT]() mutable {
				return MakeOpt<T>(bgnIt != endIt, nextT, bgnIt);
			};
			return MakeGen<T>(nextOptT);
		}

	/**
	ChildGenData struct template

	This is the Generator Data type used in conjunction with the ChainGens
	function template.
	
	Template Args:
		ParentGen (type, required): parent Generator type
		ChildData (type, required): child Generator's Data type
	
	Type Definitions:
		TParentGen: ParentGen
		TChildData: ChildData
		TParentIter: ParentGen::iterator
	
	Data Members:
		parentGen (TParentGen): instance of the parent generator
		parentIter (TParentIter): current iterator into parent generator
		childData (TChildData, optional): any extra data used by child functor
			This is only defined provided the ChildData type is not void.
			You can access it externally with myChildGen->childData.
	**/
	template<typename ParentGen, typename ChildData>
	struct ChildGenData {
		using TParentGen = ParentGen;
		using TChildData = ChildData;
		using TParentIter = typename ParentGen::iterator;

		TParentIter parentIter;
		TParentGen parentGen;
		TChildData childData;

		template<typename... ChildArgs>
			ChildGenData(TParentGen parentGen, ChildArgs&&... args):
				parentIter(parentGen.begin()),
				parentGen{std::move(parentGen)},
				childData{std::forward<ChildArgs>(args)...} {
					parentIter.mPGen = &this->parentGen;
				}
		template<typename Functor>
			static auto WrapFunctor(Functor functor) {
				return [functor](ChildGenData& cgd) mutable {
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

		TParentIter parentIter;
		TParentGen parentGen;

		ChildGenData(TParentGen parentGen):
			parentIter{parentGen.begin()},
			parentGen{std::move(parentGen)} {
				parentIter.mPGen = &this->parentGen;
			}
		template<typename Functor>
			static auto WrapFunctor(Functor functor) {
				return [fn = std::move(functor)](ChildGenData& cgd) mutable {
					return fn(cgd.parentGen, cgd.parentIter);
				};
			}
	};

	/**
	ChainGens function template

	Often with generators you want to pipe the output of one into the input of
	another. This function can help you with that.

	Example:
		Taking the example from earlier under the Generator class template,
		let's say we want the int-counting Generator to feed its output to one
		that yields the square root of anything you hand it. We'll call the
		former gen1 and the latter gen2.
		
		gen1 is the parent generator that is chained to gen2 by calling
		ChainGens. Note that while gen1 can technically stand alone, gen2
		needs gen1 in its definition because an instance of gen1 is saved within
		gen2's data.
		
		gen2's functor takes a minimum of 2 arguments: a reference to gen1 and a
		reference to a gen1 iterator marking where you are within the parent
		generator. You would typically compare this iterator to gen1.end()
		before dereferencing it.

		Source:
			auto gen1 = binon::MakeGen<int>(
				[i = 0]() mutable {
					return ++i, binon::MakeOpt<int>(i <= 5, [i] { return i; });
				});
			auto gen2 = binon::ChainGens<double>(
				std::move(gen1),
				[](auto& gen1, auto& gen1_it) {
					return binon::MakeOpt<double>(
						gen1_it != gen1.end(),
						[&gen1_it] { return std::sqrt(*gen1_it++); }
						);
				});
			for(auto x: gen2) {
				std::cout << x << '\n';
			}

		Output:
			1
			1.41421
			1.73205
			2
			2.23607

	Template Args:
		ChildT (type, required): value type child Generator produces
		ChildData (type, optional): may contain extra data for child functor
			Defaults to void, in which case the functor should only expect the 2
			required args. If you set this type to something other than void,
			you can also access it externally as myGen2->childData.
		ParentGen (type, inferred)
		Functor (type, inferred)
		ChildArgs (type, inferred)
	
	Args:
		gen (ParentGen): instance of the parent Generator
			Since the child generator will store its own instance of the parent
			Generator (within its ChildGenData structure), and in most cases any
			parent Generator instance won't be needed anymore once it has been
			incorporated into the child, you may want to move the parent into
			the child rather than copy it, as in the above example. (Here, since
			these trivial example Generators do not carry around any big
			containers, it doesn't really matter, but it's good form. By the
			way, if you can think of why it is a good idea to encapsulate the
			parent into the child rather than say just store a pointer to it,
			you get a gold star!)
		functor (Functor):
			There are 2 possible forms for this functor:
				std::optional<ChildT> fn(ParentGen&, ParentGen::iterator&)
				std::optional<ChildT>
					fn(ParentGen&, ParentGen::iterator&, ChildData&)
			The second form applies if you set the ChildData template argument
			to something other than the default void. You could also reach this
			externally with myChildGen->childData. (The parent Generator's data
			would be at *myChildGen->parentGen.)
		args (ChildArgs): extra args to initialize ChildData
	**/
	template<
		typename ChildT, typename ChildData=void,
		typename ParentGen, typename Functor, typename... ChildArgs
		>
		auto ChainGens(
			ParentGen gen, Functor functor, ChildArgs&&... args)
		{
			using CGD = ChildGenData<ParentGen,ChildData>;
			return MakeGen<ChildT,CGD>(
				CGD::WrapFunctor(std::move(functor)), std::move(gen),
				std::forward<ChildArgs>(args)...
			);
		}
}

#endif