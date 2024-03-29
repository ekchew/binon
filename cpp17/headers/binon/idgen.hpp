#ifndef BINON_IDGEN_HPP
#define BINON_IDGEN_HPP

#include "byteutil.hpp"

#include <cstdint>
#include <mutex>
#include <type_traits>
#include <vector>

namespace binon {

	/*
	kNoID constant template

	A special value that is never generated by IDGen.

	Template Args:
		ID (type, required): an unsigned integral type
	*/
	template<typename ID>
		constexpr ID kNoID = 0;

	/*
	IDGen class template

	This is the C++ counterpart to the binon.idgen.IDGen Python class. (As with
	the Python class, this implementation is also thread-safe.)

	Template Args:
		ID (type, required): an unsigned integral type

	Type Definitions:
		TID: ID
	*/
	template<typename ID>
		struct IDGen {
			static_assert(std::is_unsigned_v<ID>);

			using TID = ID;

			IDGen();

			/*
			acquire method

			Returns:
				TID: the next unused ID
			*/
			auto acquire() -> TID;

			/*
			release method

			Call this method when you are done with an ID generated earlier.

			WARNING:
				release() makes no assertions that an ID has not been discarded
				already. It is up to you to track your ID life cycles lest the
				uniqueness logic break down.

			Note that you can use a NewID lock to manage the releasing for you
			automatically.

			Args:
				theID (TID): the ID to discard
			*/
			void release(TID theID);

		 private:
			std::vector<std::byte> mFreeIDs;
			TID mOflwID;
			TID mOflwCnt;
			std::mutex mMutex;
		};

	/*
	NewID class template

	This class is roughly analogous to its Python counterpart. You declare an
	instance of it as a local variable and it will retain ownership of a new ID
	until the instance goes out of scope and its destructor releases the ID.

	To access the ID itself, you can either call getValue() or let implicit
	conversion to the ID type do the work.

	(Note that the NewID class is moveable but not copyable or
	default-constructible.)

	Type Definitions:
		ID (type, inferred)
	*/
	template<typename ID>
		struct NewID {

			/*
			constructor

			Args:
				idGen (IDGen<ID> L-value):
					Clearly, the IDGen instance must exist for at least as long
					as the NewID instance does.
			*/
			NewID(IDGen<ID>& idGen);

			NewID(const NewID&) = delete;
			NewID(NewID&& newID) noexcept;
			NewID() noexcept = default;
			auto operator = (const NewID&) -> NewID& = delete;
			auto operator = (NewID&& newID) noexcept -> NewID&;

			/*
			operator ID, getValue method:

			Returns:
				ID: the new ID
			*/
			operator ID() const noexcept { return mID; }
			auto getValue() const noexcept -> ID { return mID; }

			~NewID();

		 private:
			IDGen<ID>* mPIDGen = nullptr;
			ID mID = kNoID<ID>;
		};

	//==== Template Implementation =============================================

	//---- IDGen ---------------------------------------------------------------

	template<typename ID>
		IDGen<ID>::IDGen():
			mFreeIDs(0x7f),
			mOflwID{0x80},
			mOflwCnt{0}
		{
			auto i = mFreeIDs.size();
			for(auto&& b: mFreeIDs) {
				b = ToByte(i--);
			}
		}
	template<typename ID>
		auto IDGen<ID>::acquire() -> TID {
			std::lock_guard<decltype(mMutex)> lg{mMutex};
			TID theID;
			if(mFreeIDs.empty()) {
				if(mOflwCnt++ == 0) {
					mOflwID = 0x80;
				}
				theID = mOflwID++;
			}
			else {
				theID = std::to_integer<TID>(mFreeIDs.back());
				mFreeIDs.pop_back();
			}
			return theID;
		}
	template<typename ID>
		void IDGen<ID>::release(TID theID) {
			std::lock_guard<decltype(mMutex)> lg{mMutex};
			if(theID < 0x80u) {
				mFreeIDs.push_back(ToByte(theID));
			}
			else {
				--mOflwCnt;
			}
		}

	//---- NewID ---------------------------------------------------------------

	template<typename ID>
		NewID<ID>::NewID(IDGen<ID>& idGen):
			mPIDGen{&idGen},
			mID{idGen.acquire()}
		{
		}
	template<typename ID>
		NewID<ID>::NewID(NewID&& newID) noexcept:
			mPIDGen{newID.mPIDGen},
			mID{newID.mID}
		{
			newID.mID = kNoID<ID>;
		}
	template<typename ID>
		auto NewID<ID>::operator = (NewID&& newID) noexcept -> NewID& {
			mPIDGen = newID.mPIDGen;
			mID = newID.mID;
			newID.mID = kNoID<ID>;
			return *this;
		}
	template<typename ID>
		NewID<ID>::~NewID()
		{
			if(mID != kNoID<ID>) {
				mPIDGen->release(mID);
				mID = kNoID<ID>;
			}
		}
}

#endif
