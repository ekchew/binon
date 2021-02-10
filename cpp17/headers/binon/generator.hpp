#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "macros.hpp"
#include "optutil.hpp"

#include <cstddef>
#include <functional>
#include <iterator>
#include <type_traits>

namespace binon {

	/**
	GenBase class template

	This parent class of Generator has a specialization for dealing with the
	Data type void. See the Generator class and MakeGen function template
	docs for more info.

	Template Args:
		T (type): see Generator
		Data (type): see Generator
		Fn (type): see Generator
	**/
	namespace details {
		//	GenBase is not actually the absolute base class of Generator, as it
		//	has a trivial parent class GenType. GenType is not a template, which
		//	makes it easy to check if a given type T is actually a Generator, as
		//	is done in a few internal "if constexpr" statements here and there.
		class GenType{};
	}
	template<typename T, typename Data, typename Fn>
		struct GenBase: details::GenType {
			/**
			constructor

			GenBase's constructor is exposed by Generator.

			Template Args:
				TFn (inferred)
				DataArgs (inferred)

			Args:
				functor (TFn): your functor
				dataArgs... (DataArgs...): args to initialize a Data instance
					Clearly, these only make sense if your Data type is not
					void.
			**/
			template<typename TFn, typename... DataArgs>
				GenBase(TFn&& functor, DataArgs&&... dataArgs):
					mFunctor{std::forward<TFn>(functor)},
					mData{std::forward<DataArgs>(dataArgs)...} {}

			/**
			operator overloads: *

			This operator gives you access to the internal Data instance (the
			same one that is passed to your functor by reference). It is only
			defined when your Data type is something other than void.
			
			There are 2 overloads: one gives you an L-value reference and the
			other an R-value reference. You can go *std::move(gen) to get the
			latter. (You probably wouldn't want to do so before you're done
			iterating though.)
			**/
			auto operator * () & noexcept -> Data& { return mData; }
			auto operator * () && noexcept -> Data&& { return std::move(mData); }

			/**
			operator overload: ->

			If your Data type is a class/struct, you can access members with ->.
			It is only defined when your Data type is something other than void.
			**/
			auto operator -> () noexcept -> Data* { return &mData; }

		 protected:
			Fn mFunctor;
			Data mData;

			auto callFunctor() -> std::optional<T> { return mFunctor(mData); }
	};
	template<typename T, typename Fn>
		struct GenBase<T, void, Fn>: details::GenType {
			static_assert(
				std::is_same_v<
					std::invoke_result_t<Fn>, std::optional<T>
					>,
				"Generator functor must take the form: std::optional<T> fn()"
				);

			template<typename TFn>
				GenBase(TFn&& functor) noexcept:
					mFunctor{std::forward<TFn>(functor)} {}

		 protected:
			Fn mFunctor;

			auto callFunctor() -> std::optional<T> { return mFunctor(); }
		};

	/**
	Generator class template

	A Generator is an iterable class built around a functor you supply that
	returns a series of values over multiple calls. Unless you know what you're
	doing, you will want to call MakeGen (or another helper function like
	PipeGen) to instantiate this class.

	In its simpler form, the functor would look like this:

		std::optional<T> myFunctor();

	It would return optional T values until they are exhausted, at which point
	it would return the null option.

	The above form applies when the Data template argument is set to the default
	void. If you set it to something other than void, Generator will store an
	internal instance of that type and pass it to your functor by reference.

		std::optional<T> myFunctor(Data& data); // for non-void Data type

	Example 1: Print integers from 1 to 5
		Functors are typically implemented as lambda functions. This example
		demonstrates one way you could have a variable that changes every time
		your lambda gets called: in this case, the integer counter i. We
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
		This time, we tell MakeGen to store an int (2nd template arg) we
		initialize to 0 (2nd function arg). Then an int& is supplied to the
		functor and we can increment it as in the first example.

		One advantage of having the Generator store your data is that you can
		access it externally (not just from within your functor) by using the
		* operator (or -> if your Data type is a struct). Here we see that i
		climbs all the way to 6 by the time the iteration loop ends.

		Source:
			auto gen = binon::MakeGen<int, int>(
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
			Type-defined to TValue and value_type.
		Data (type, required): optional extra data for functor
			Type-defined to TData.
			Set to void if your functor needs no extra data. (See also
			MakeGen function template.)
		Fn (type, inferred):
			Type-defined to TFunctor.
			Takes one of 2 forms:
				std::optional<T> fn()
				std::optional<T> fn(Data&)
			The 1st form applies if the Data type is void.
	**/
	template<typename T, typename Data, typename Fn>
		struct Generator: GenBase<T,Data,Fn> {
			using TValue = T;
			using TData = Data;
			using TFunctor = Fn;
			using value_type = TValue;

			/**
			iterator class

			This is an input iterator instantiated by Generator's begin and end
			methods. It stores an instance of the last std::optional<T> returned
			by your functor, which is updated by the ++ operator. You can reach
			the T value by dereferencing the iterator with * or ->. In debug
			mode, this may throw std::bad_optional_access if there is no value.
			(In release mode, this check is suppressed for performance.)
			
			There is no const_iterator class. That means you could mess around
			with the iterator's value, but what's the point when it's going to
			get overwritten by ++ anyway? The * operator does, however, support
			move semantics. If you were generating std::string, for example, you
			could avoid a needless copy with:

				auto myStr = *std::move(myIt);

			(Implementation Note: iterator is smart about
			std::reference_wrapper. If your TValue is a wrapper, it will unwrap
			it and give you a plain L-value reference when you dereference the
			iterator. This should eliminate any drama when you use the ->
			operator, for example.)
			**/
			struct iterator {
				using iterator_category = std::input_iterator_tag;
				using value_type = Generator::value_type;
				using difference_type = std::ptrdiff_t;
				using reference = TUnwrappedRef<value_type>;
				using pointer = value_type*;

				Generator* mPGen;
				std::optional<value_type> mOptVal;

				explicit iterator(Generator& gen) noexcept:
					mPGen{&gen} {}
				explicit operator bool() const noexcept
					{ return mOptVal.has_value(); }
				auto operator == (const iterator& rhs) const noexcept -> bool
					{ return EqualOpts(mOptVal, rhs.mOptVal); }
				auto operator != (const iterator& rhs) const noexcept -> bool
					{ return !EqualOpts(mOptVal, rhs.mOptVal); }
				auto operator * () & -> reference
					{ return DerefOpt(mOptVal); }
				auto operator * () && -> TRefBase<value_type>&&
					{ return DerefOpt(std::move(mOptVal)); }
				auto operator -> () -> pointer
					{ return &BINON_IF_DBG_REL(mOptVal.value(), *mOptVal); }
				auto operator ++ () -> iterator&
					{ return mOptVal = mPGen->callFunctor(), *this; }
				auto operator ++ (int) -> iterator {
						iterator copy{*this};
						return ++*this, copy;
					}
			};

			using GenBase<T,Data,Fn>::GenBase;

			/**
			begin method

			Returns:
				iterator
			**/
			auto begin() -> iterator { return ++iterator{*this}; }
			/**
			end method

			Returns:
				iterator: stores std::null_opt internally
			**/
			auto end() -> iterator { return iterator{*this}; }
		};

	/**
	MakeGen function template

	Builds a Generator instance around the type it yields and a functor you
	supply (plus any extra data for your functor if you request it).

	Template Args:
		T (type, required): the value type the generator produces
		Data (type, optional): defaults to void
		Fn (type, inferred): see Generator
		DataArgs (types, inferred)

	Args:
		functor (Fn): see Generator
		dataArgs... (DataArgs...): any extra args to initialize a Data instance
			Clearly, these only make sense if your Data type is not void.

	Returns:
		Generator<T,Data,Fn>
	**/
	template<
		typename T, typename Data=void,
		typename Fn, typename... DataArgs
		>
		auto MakeGen(Fn&& functor, DataArgs&&... dataArgs) {

			//	If functor is passed in by L-value, Fn will be reference type,
			//	and we don't want that for the Generator type since the functor
			//	may lose any captured data if it is not fully copied into the
			//	Generator.
			using TFn = std::remove_reference_t<Fn>;

			return Generator<T,Data,TFn>{
				std::forward<Fn>(functor),
				std::forward<DataArgs>(dataArgs)...};
		}

	/**
	IterGen class template

	This class simply builds a Generator-like object around a begin/end iterator
	pair you supply. You typically used it for the parent class when you want to
	call PipeGen/PipeGenVals with the parent being some sort of container.

	You could pass the container instance itself into PipeGen, but this may
	cause PipeGen to copy the entire container into the local storage of the
	returned Generator (unless you use move semantics). IterGen only contains an
	iterator pair, so it should be much lighter-weight than a container.

	Example:
		Say you wanted a Generator that square every element in a
		std::vector<int>.

		Source:
			std::vector<int> v = {1, 2, 3};
			auto squared = binon::PipeGenVals<int>(
				binon::IterGen{v.begin(), v.end()},
				[](int i) { return i * i; });
			for(int i: squared) {
				std::cout << i << '\n';
			}

		Output:
			1
			4
			9

	Template Args:
		BgnIt (type, inferred): type-defined to TBgnIt
		EndIt (type, inferred): type-defined to TEndIt

	Args:
		bgnIt (BgnIt): begin iterator
		endIt (EndIt): end iterator

	Returns:
		Generator of BgnIt::value_type
	**/
	template<typename BgnIt, typename EndIt>
		struct IterGen {
			using TBgnIt = BgnIt;
			using TEndIt = EndIt;
			using iterator = TBgnIt;
			IterGen(BgnIt bgnIt, EndIt endIt) noexcept:
				mBgnIt{std::move(bgnIt)}, mEndIt{std::move(endIt)} {}
			constexpr auto begin() -> TBgnIt& { return mBgnIt; }
			constexpr auto begin() const -> const TBgnIt& { return mBgnIt; }
			constexpr auto end() -> TEndIt& { return mEndIt; }
			constexpr auto end() const -> const TEndIt& { return mEndIt; }
		 private:
			TBgnIt mBgnIt;
			TEndIt mEndIt;
		};

	/**
	ChildGenData struct template

	This is the child Generator Data type used in conjunction with PipeGen and
	PipeGenVals.

	Template Args:
		ParentGen (type, required): parent Generator type
		ChildData (type, required): child Generator's Data type

	Type Definitions:
		TParentGen: ParentGen
			Type-defined to TParentGen.
		TChildData: ChildData
			Type-defined to TChildData.
		TParentIter: ParentGen::iterator
			Type-defined to TParentIter.

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

			/**
			constructor

			Template Args:
				ChildArgs (types, inferred)

			Args:
				parentGen (TParentGen): parent generator
					Technically, this can be any input iterable type, but if
					it's not a Generator, read up on IterGen as it may be a
					suitable alternative.
				args (ChildArgs): init args for ChildData type if not void
			**/
			template<typename... ChildArgs>
				ChildGenData(TParentGen parentGen, ChildArgs&&... args):
					parentIter(parentGen.begin()),
					parentGen{std::move(parentGen)},
					childData{std::forward<ChildArgs>(args)...} {
						if constexpr(
							std::is_base_of_v<details::GenType, TParentGen>)
						{
							parentIter.mPGen = &this->parentGen;
						}
					}

			/**
			WrapFunctor class method template

			What PipeGen's functor expects and what the Generator it returns
			tries to give it are not quite the same thing. WrapFunctor returns a
			functor that smooths over these differences.

			Template Args:
				Fn (type, inferred)

			Args:
				functor (Fn): the functor given to PipeGen

			Returns:
				functor: the functor PipeGen assigns to the Generator it returns
			**/
			template<typename Fn>
				static auto WrapFunctor(Fn&& functor) {
					using TFn = std::remove_reference_t<Fn>;
					return [fn = TFn{std::forward<Fn>(functor)}](
						ChildGenData& cgd) mutable
					{
						return fn(
							cgd.parentGen, cgd.parentIter, cgd.childData);
					};
				}

			template<typename T, typename Fn>
				static auto ValsFunctor(Fn&& functor) {
					using TFn = std::remove_reference_t<Fn>;
					return [fn = TFn{std::forward<Fn>(functor)}](
						TParentGen& parGen, TParentIter& parIt,
						TChildData& chdData
						) mutable -> std::optional<T>
					{
						if(parIt == parGen.end()) {
							return std::nullopt;
						}
						else {
							return std::make_optional<T>(
								fn(*std::move(parIt)++, chdData));
						}
					};
				}
			template<typename T, typename Fn>
				static auto RefsFunctor(Fn&& functor) {
					using TFn = std::remove_reference_t<Fn>;
					return [fn = TFn{std::forward<Fn>(functor)}](
						TParentGen& parGen, TParentIter& parIt,
						TChildData& chdData) mutable -> std::optional<T>
					{
						if(parIt == parGen.end()) {
							return std::nullopt;
						}
						else {
							return std::make_optional<T>(fn(*parIt++, chdData));
						}
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
					if constexpr(
						std::is_base_of_v<details::GenType, TParentGen>)
					{
						parentIter.mPGen = &this->parentGen;
					}
				}
			template<typename Fn>
				static auto WrapFunctor(Fn&& functor) {
					using TFn = std::remove_reference_t<Fn>;
					return [fn = TFn{std::forward<Fn>(functor)}](
						ChildGenData& cgd) mutable
					{
						return fn(cgd.parentGen, cgd.parentIter);
					};
				}
			template<typename T, typename Fn>
				static auto ValsFunctor(Fn&& functor) {
					if constexpr(
						std::is_base_of_v<details::GenType, TParentGen>)
					{
						using TFn = std::remove_reference_t<Fn>;
						return [fn = TFn{std::forward<Fn>(functor)}](
							TParentGen& parGen, TParentIter& parIt) mutable
							-> std::optional<T>
						{
							if(parIt == parGen.end()) {
								return std::nullopt;
							}
							else {
								return std::make_optional<T>(
									fn(*std::move(parIt)++));
							}
						};
					}
					else {
						return RefsFunctor<T,Fn>(std::forward<Fn>(functor));
					}
				}
			template<typename T, typename Fn>
				static auto RefsFunctor(Fn&& functor) {
					using TFn = std::remove_reference_t<Fn>;
					return [fn = TFn{std::forward<Fn>(functor)}](
						TParentGen& parGen, TParentIter& parIt) mutable
						-> std::optional<T>
					{
						if(parIt == parGen.end()) {
							return std::nullopt;
						}
						else {
							return std::make_optional<T>(fn(*parIt++));
						}
					};
				}
		};

	/**
	PipeGen function template

	Often with generators you want to pipe the output of one into the input of
	another. This function can help you with that by returning a new "child"
	Generator that takes its input from a "parent".

	Note that PipeGen stores a full instance of the parent generator within the
	child. That way, you shouldn't normally need to worry about the parent being
	destructed before the child is done with it. But in some cases, this can be
	a bit of overkill. You may be iterating a container of some sort and do not
	want all the container data copied into the child generator. In that case,
	you can use an IterGen instance built from the container is the parent.

	See also PipeGenVals.

	Template Args:
		ChildT (type, required): value type child Generator produces
		ChildData (type, optional): may contain extra data for child functor
			Defaults to void, in which case the functor should only expect the 2
			required args. If you set this type to something other than void,
			you can also access it externally as myGen2->childData.
		ParentGen (type, inferred)
		Fn (type, inferred)
		ChildArgs (type, inferred)

	Args:
		gen (ParentGen): instance of the parent generator
			ParentGen need not necessarily be an instance of Generator. Pretty
			much any input iterable type should work here.
		functor (Fn):
			There are 2 possible forms for this functor:
				std::optional<ChildT>
					fn(ParentGen&, ParentGen::iterator&)
				std::optional<ChildT>
					fn(ParentGen&, ParentGen::iterator&, ChildData&)
			The second form applies if you set the ChildData template argument
			to something other than the default void. You could also reach this
			externally with myChildGen->childData. (The parent Generator's data,
			if available, would be at *myChildGen->parentGen.)
		args (ChildArgs): extra args to initialize ChildData
	**/
	template<
		typename ChildT, typename ChildData=void,
		typename ParentGen, typename Fn, typename... ChildArgs
		>
		auto PipeGen(
			ParentGen gen, Fn functor, ChildArgs&&... args)
		{
			using CGD = ChildGenData<ParentGen,ChildData>;
			return MakeGen<ChildT,CGD>(
				CGD::WrapFunctor(std::move(functor)), std::move(gen),
				std::forward<ChildArgs>(args)...);
		}

	/**
	PipeGenVals function template

	This is a higher level alternative to PipeGen in which your functor no
	longer has to worry about managing iterators. It's called repeated with a
	dereferenced iterator from the parent generator until all of the parent's
	output is exhausted. It is therefore useful when you simply want to filter
	the parent's output in some way, since there is a one-to-one correspondence
	between the values the parent and child yield.

	See IterGen for a simple usage example.

	Template Args:
		ChildT (type, required): value type child Generator produces
		ChildData (type, optional): may contain extra data for child functor
			Defaults to void, in which case the functor should only expect the 2
			required args. If you set this type to something other than void,
			you can also access it externally as myGen2->childData.
		ParentGen (type, inferred)
		Fn (type, inferred)
		ChildArgs (type, inferred)

	Args:
		gen (ParentGen): instance of the parent Generator
			See PipeGen for more info.
		functor (Fn):
			There are 2 possible forms for this functor:
				std::optional<ChildT>
					fn(ParentGen::TValue)
				std::optional<ChildT>
					fn(ParentGen::TValue, ChildData&)
			Note that these arguments can be received as const references or
			even by value depending on what your child generator wants to do
			with them.
		args (ChildArgs): extra args to initialize ChildData
	**/
	template<
		typename ChildT, typename ChildData=void,
		typename ParentGen, typename Fn, typename... ChildArgs
		>
		auto PipeGenVals(
			ParentGen gen, Fn&& functor, ChildArgs&&... args)
		{
			using CGD = ChildGenData<ParentGen,ChildData>;
			return PipeGen<ChildT,ChildData>(
				gen,
				CGD::template ValsFunctor<ChildT>(std::forward<Fn>(functor)),
				std::forward<ChildArgs>(args)...);
		}
	template<
		typename ChildT, typename ChildData=void,
		typename ParentGen, typename Fn, typename... ChildArgs
		>
		auto PipeGenRefs(
			ParentGen gen, Fn&& functor, ChildArgs&&... args)
		{
			using CGD = ChildGenData<ParentGen,ChildData>;
			return PipeGen<ChildT,ChildData>(
				gen,
				CGD::template RefsFunctor<ChildT>(std::forward<Fn>(functor)),
				std::forward<ChildArgs>(args)...);
		}
}

#endif
