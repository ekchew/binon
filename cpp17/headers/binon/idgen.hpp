#ifndef BINON_IDGEN_HPP
#define BINON_IDGEN_HPP

#include "byteutil.hpp"

#include <cstdint>
#include <mutex>
#include <vector>

namespace binon {

	/**
	IDGenData class template

	This class is functionally equivalent to the IDGen class from the idgen
	Python module except that it is not iterable. Call MakeIDGen() instead to
	give you an iterable object.

	Template Args:
		ID (type, required): an unsigned integral type

	Type Definitions:
		TID: ID
	**/
	template<typename ID>
		struct IDGenData {
			static_assert(std::is_unsigned_v<ID>);

			using TID = ID;

			IDGenData();

			/**
			acquire method

			Returns:
				TID: the next unused ID
			**/
			auto acquire() -> TID;

			/**
			discard method

			Call this method when you are done with an ID generated earlier.

			WARNING:
				discard() makes no assertions that an ID has not been discarded
				already. It is up to you to track your ID life cycles lest the
				uniqueness logic break down.

			Args:
				theID (TID): the ID to discard
			**/
			void discard(TID theID);

		 private:
			std::vector<std::byte> mFreeIDs;
			TID mOflwID;
			TID mOflwCnt;
			std::mutex mMutex;
		};

	/**
	MakeIDGen function template:

	This function returns a binon::Generator that is nearly functionally
	equivalent to the IDGen class from the idgen Python module. (Because of the
	way Generator classes cache one value in their iterators, discarded IDs take
	one extra iteration before they become available again -- hence the
	"nearly".) As described in the module, this generator yields unsigned
	integer ID codes in such a way that you can discard ones you no longer need
	to be re-used. This can keep the numbers small so that they encode in fewer
	bytes as BinON UIntObjs.

	Note that as with its Python counterpart, this generator (and the IDGenData
	class it is built atop) is thread-safe.

	Example:

		Source:
			auto gen = binon::MakeIDGen();
			auto it = gen.begin();
			auto idA = *it++;
			auto idB = *it++;
			gen.discard(idA);
			auto idC = *it++; // C gets value cached in "it"
			auto idD = *it++; // D gets recycled A value
			std::cout << idB << ' ' << idC << ' ' << idD << '\n';

		Output:
			2 3 1

	Template Args:
		ID (type, optional): defaults to std::uint64_t
			This needs to be an unsigned integral type.

	Returns:
		Generator of ID
	**/
	template<typename ID=std::uint64_t>
		auto MakeIDGen() {
			return MakeGen<ID,IDGenData<ID>>(
				[](IDGenData<ID>& data) {
					return std::make_optional<ID>(data.acquire());
				});
		}

	//==== Template Implementation =============================================

	//---- IDGenData -----------------------------------------------------------

	template<typename ID>
		IDGenData<ID>::IDGenData():
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
		auto IDGenData<ID>::acquire() -> TID {
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
		void IDGenData<ID>::discard(TID theID) {
			std::lock_guard<decltype(mMutex)> lg{mMutex};
			if(theID < 0x80u) {
				mFreeIDs.push_back(ToByte(theID));
			}
			else {
				--mOflwCnt;
			}
		}
}

#endif
