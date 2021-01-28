#ifndef BINON_PBINONOBJT_HPP
#define BINON_PBINONOBJT_HPP

#include "listobj.hpp"

namespace binon {

	/**
	PBinONObj class

	This is the abstract base class of PBinONObjT. It lets you access the
	latter's shared pointer as a generic TSPBinONObj to any type of BinONObj.
	**/
	struct PBinONObj {

		/**
		getBasePtr method

		Returns:
			TSPBinONObj: the pointer from the PBinONObjT subclass
		**/
		virtual auto getBasePtr() noexcept -> TSPBinONObj = 0;
		auto getBasePtr() const noexcept -> const TSPBinONObj {
				return const_cast<PBinONObj*>(this)->getBasePtr();
			}

		virtual ~PBinONObj() = default;
	};

	/**
	PBinONObjT class template

	When you are dealing with a container of variable element types (e.g.
	ListObj), what you will see is a bunch of TSPBinONObj values: a rather
	opaque data type representing a shared pointer to the BinONObj base class.
	Assuming you know what the element really is (calling myElem.typeCode() may
	shed some light on this if there is any uncertainty), you can wrap a
	PBinONObjT around it to get at the actual value.

	Example 1: Get int from list element
		Here we create a PBinONObjT of IntObj by calling FromPObj on the element
		(in its original TSPBinONObj form). Then we dereference it to get the
		IntObj, which we cast to a simple int for printing.

		Source:
			auto pElem = binon::PBinONObjT<binon::IntObj>::FromPObj(myElem);
			std::cout << "myElem has the value: "
				<< static_cast<int>(*pElem) << '\n';

	Example 2: Simpler version of Example 1
		For a simple scalar type like int, PBinONObjT can deduce that it would
		be represented by a binon::IntObj. The pointer stored within pElem will
		still be to an IntObj for the purposes of dereferencing it with the *
		operator, but you can call the getVal() method to see it in the form of
		the int you specified as a template argument instead.

		Source:
			auto pElem = binon::PBinONObjT<int>::FromPObj(myElem);
			std::cout << "myElem has the value: " << pElem.getVal() << '\n';


	Example 3: Add an IntObj to a ListObj
		On the encoding side, you can call the Make class method to allocate a
		shared pointer to your value, which you can then add to your ListObj.

		Source:
			auto pElem = binon::PBinONObjT<binon::IntObj>::Make(42);
			myList.pushBack(pElem.getPtr());

	Template Args:
		T (required): value type

	Type Definitions:
		TVal: T
		TObj: BinON object type corresponding to T
		TPObj: shared pointer to TObj

	Data Members:
		mPObj (TPObj): dynamically allocated BinON object
	**/
	template<typename T>
		struct PBinONObjT: PBinONObj {

			//---- Type Definitions --------------------------------------------

			using TVal = T;
			using TObj = TWrapper<TVal>;
			using TPObj = std::shared_ptr<TObj>;

			//---- Class Methods -----------------------------------------------

			/**
			Make class method template

			Calling Make is the easiest way to build a new PBinONObjT from
			scratch for encoding purposes.

			Template Args:
				Args (types, inferred)

			Args:
				args (Args, optional): args used to initialize a new TVal

			Returns:
				PBinONObjT<TVal>
			**/
			template<typename... Args>
				static auto Make(Args&&... args) -> PBinONObjT;

			/**
			FromPObj class method

			Calling FromPObj is the easiest way to wrap a PBinONObjT around a
			TSPBinONObj you have decoded.

			Args:
				pObj (TSPBinONObj or const TSPBinONObj):
					This is an existing shared pointer to a BinON object.

			Returns:
				PBinONObjT<TVal> or const PBinONObjT<TVal>:
					Returns the const form if pObj was const.
			**/
			static auto FromPObj(TSPBinONObj& pObj) -> PBinONObjT;
			static auto FromPObj(const TSPBinONObj& pObj) -> const PBinONObjT;

			//---- Public Instance Methods -------------------------------------

			/**
			operator bool method

			Returns:
				bool: true if mPObj is allocated
			**/
			operator bool() const;

			/**
			operator * overload

			Dereferences mPObj. In debug mode, this may throw binon::NullDeref
			if the pointer is null.

			Returns:
				TObj& or const TObj&: const if PBinONObjT instance is const
			**/
			auto operator * () -> TObj&;
			auto operator * () const -> const TObj&;

			/**
			operator -> overload

			Dereferences mPObj. In debug mode, this may throw binon::NullDeref
			if the pointer is null.

			Returns:
				TObj* or const TObj*: const if PBinONObjT instance is const
			**/
			auto operator -> () -> TObj*;
			auto operator -> () const -> const TObj*;

			/**
			getPtr method

			This simply returns mPObj.

			Returns:
				TObj* or const TObj*: const if PBinONObjT instance is const
			**/
			auto getPtr() -> TPObj;
			auto getPtr() const -> const TPObj;

			/**
			getVal method

			Dereferences mPObj and casts it to TVal. In debug mode, this may
			throw binon::NullDeref if the pointer is null.

			Returns:
				TVal: the type you specified as the template arg
			**/
			auto getVal() const -> TVal;

			/**
			setVal method

			This method lets you assign the value stored in mPObj. If mPObj has
			yet to be allocated, setVal will do so. Note that this method
			supports move semantics.

			Args:
				val (TVal): the value to assign
			**/
			void setVal(const TVal& val);
			void setVal(TVal&& val);

			/**
			emplaceVal method template

			Instantiates a new TVal using any arguments you provide and assigns
			it to the value stored in mPObj. If mPObj has yet to be allocated,
			emplaceVal will do so.

			Template Args:
				Args (types, inferred)

			Args:
				args (Args, optional): args to initialize a new TVal
			**/
			template<typename... Args>
				void emplaceVal(Args&&... args);
			
			auto getBasePtr() noexcept -> TSPBinONObj final { return mPObj; }

		private:

			//---- Data Members ------------------------------------------------

			TPObj mPObj;

			//---- Private Instance Methods ------------------------------------

			void assertPtr() const;
		};

	//==== Template Implementation =============================================

	template<typename T>
	template<typename... Args>
		auto PBinONObjT<T>::Make(Args&&... args) -> PBinONObjT {
			return {std::make_shared<TObj>(std::forward<Args>(args)...)};
		}
	template<typename T>
		auto PBinONObjT<T>::FromPObj(TSPBinONObj& pObj) -> PBinONObjT {
			return {BinONObj::Cast<TObj>(pObj)};
		}
	template<typename T>
		auto PBinONObjT<T>::FromPObj(const TSPBinONObj& pObj)
			-> const PBinONObjT
		{
			return FromPObj(const_cast<TSPBinONObj&>(pObj));
		}
	template<typename T>
		PBinONObjT<T>::operator bool() const {
			return mPObj;
		}
	template<typename T>
		auto PBinONObjT<T>::operator * () -> TObj& {
			BINON_IF_DEBUG(assertPtr();)
			return *mPObj;
		}
	template<typename T>
		auto PBinONObjT<T>::operator * () const -> const TObj& {
			return *const_cast<PBinONObjT*>(this);
		}
	template<typename T>
		auto PBinONObjT<T>::operator -> () -> TObj* {
			BINON_IF_DEBUG(assertPtr();)
			return mPObj.get();
		}
	template<typename T>
		auto PBinONObjT<T>::operator -> () const -> const TObj* {
			BINON_IF_DEBUG(assertPtr();)
			return mPObj.get();
		}
	template<typename T>
		auto PBinONObjT<T>::getPtr() -> TPObj {
			return mPObj;
		}
	template<typename T>
		auto PBinONObjT<T>::getPtr() const -> const TPObj {
			return mPObj;
		}
	template<typename T>
		auto PBinONObjT<T>::getVal() const -> TVal {
			BINON_IF_DEBUG(assertPtr();)
			return static_cast<TVal>(mPObj->mValue);
		}
	template<typename T>
		void PBinONObjT<T>::setVal(const TVal& val) {
			if(mPObj) {
				mPObj->mValue = val;
			}
			else {
				mPObj = Make(val);
			}
		}
	template<typename T>
		void PBinONObjT<T>::setVal(TVal&& val) {
			using std::move;
			if(mPObj) {
				mPObj->mValue = move(val);
			}
			else {
				mPObj = Make(move(val));
			}
		}
	template<typename T>
	template<typename... Args>
		void PBinONObjT<T>::emplaceVal(Args&&... args) {
			using std::forward;
			if(mPObj) {
				mPObj->mValue = TObj(forward<Args>(args)...);
			}
			else {
				mPObj = Make(forward<Args>(args)...);
			}
		}
	template<typename T>
		void PBinONObjT<T>::assertPtr() const {
			if(!mPObj) {
				throw NullDeref{"binon::PBinONObjT has no value"};
			}
		}
}

#endif
