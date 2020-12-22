#include "binon/listobj.hpp"
#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"

#include <sstream>
#if BINON_LIB_EXECUTION
	#include <execution>
	#define BINON_PAR_UNSEQ std::execution::par_unseq,
#else
	#pragma message "C++17 execution policies unavailable"
	#define BINON_PAR_UNSEQ
#endif

namespace binon {
	
	auto DeepCopyTList(const TList& list) -> TList {
		TList copy(list.size());
		std::transform(BINON_PAR_UNSEQ
			list.begin(), list.end(), copy.begin(),
			[](const TList::value_type& p)
				{ return p->makeCopy(kDeepCopy); }
			);
		return std::move(copy);
	}
	
	//---- ListObj -------------------------------------------------------------
	
	void ListObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		UInt count{mValue.size()};
		count.encodeData(stream, kSkipRequireIO);
		encodeElems(stream, kSkipRequireIO);
	}
	void ListObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt count;
		count.decodeData(stream, kSkipRequireIO);
		decodeElems(stream, count, kSkipRequireIO);
	}
	void ListObj::encodeElems(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		for(auto&& pObj: mValue) {
			pObj->encode(stream, kSkipRequireIO);
		}
	}
	void ListObj::decodeElems(
		TIStream& stream, TValue::size_type count, bool requireIO)
	{
		RequireIO rio{stream, requireIO};
		mValue.resize(count);
		for(auto&& pObj: mValue) {
			pObj = Decode(stream, kSkipRequireIO);
		}
	}
	auto ListObj::hasDefVal() const -> bool {
		return mValue.size() == 0;
	}
	auto ListObj::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<ListObj>(DeepCopyTList(mValue));
		}
		return std::make_shared<ListObj>(*this);
	}
	
	//---- SList ---------------------------------------------------------------
	
	auto SList::typeCode() const noexcept -> CodeByte {
		return kSListCode;
	}
	void SList::assertElemTypes() const {
		auto n = mValue.mList.size();
		for(decltype(n) i = 0; i < n; ++i) {
			auto tc = mValue.mList[i]->typeCode();
			if(tc != mValue.mElemCode) {
				std::ostringstream oss;
				auto hex0 = AsHex(mValue.mElemCode);
				auto hex1 = AsHex(tc);
				oss	<< "SList element " << i << " type code 0x" << hex1
					<< " does not match expected 0x" << hex0;
				throw TypeErr{oss.str()};
			}
		}
	}
	void SList::encodeData(TOStream& stream, bool requireIO) const {
		BINON_IF_DEBUG(assertElemTypes();)
		RequireIO rio{stream, requireIO};
		UInt count{mValue.mList.size()};
		count.encodeData(stream, kSkipRequireIO);
		encodeElems(stream, kSkipRequireIO, kSkipAssertTypes);
	}
	void SList::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt count;
		count.decodeData(stream, kSkipRequireIO);
		mValue.mList.resize(count);
		decodeElems(stream, count, kSkipRequireIO);
	}
	void SList::encodeElems(
		TOStream& stream, bool requireIO, bool assertTypes) const
	{
		if(assertTypes) {
			assertElemTypes();
		}
		
		//	Write element code.
		RequireIO rio{stream, requireIO};
		mValue.mElemCode.write(stream, kSkipRequireIO);
		
		//	Write data of all elements consecutively.
		if(mValue.mElemCode.baseType() == kBoolObjCode.baseType()) {
			
			//	Special case for booleans which get packed 8 to a byte.
			std::byte byt = 0x00_byte;
			auto n = mValue.mList.size();
			decltype(n) i;
			for(i = 0; i < n; ++i) {
				auto&& pObj = mValue.mList[i];
				byt <<= 1;
				if(!pObj->hasDefVal()) {
					byt |= 0x01_byte;
				}
				if((i & 0x7) == 0x7) {
					WriteWord(byt, stream, kSkipRequireIO);
					byt = 0x00_byte;
				}
			}
			if(i & 0x7) {
				byt <<= 8 - i;
				WriteWord(byt, stream, kSkipRequireIO);
			}
		}
		else {
			for(auto&& pObj: mValue.mList) {
				pObj->encodeData(stream, kSkipRequireIO);
			}
		}
	}
	void SList::decodeElems(
		TIStream& stream, TList::size_type count, bool requireIO)
	{
		//	Read element code.
		RequireIO rio{stream, requireIO};
		mValue.mElemCode = CodeByte::Read(stream, kSkipRequireIO);
		auto pBaseObj = FromTypeCode(mValue.mElemCode);
		
		//	Read data of all elements consecutively.
		mValue.mList.resize(count);
		if(mValue.mElemCode.baseType() == kBoolObjCode.baseType()) {
			
			//	Special case for booleans packed 8 to a byte.
			std::byte byt = 0x00_byte;
			for(decltype(count) i = 0; i < count; ++i) {
				if((i & 0x7) == 0) {
					byt = ReadWord<decltype(byt)>(stream, kSkipRequireIO);
				}
				mValue.mList[i] = std::make_shared<BoolObj>(
					(byt & 0x80_byte) != 0x00_byte);
				byt <<= 1;
			}
		}
		else {
			for(auto&& pObj: mValue.mList) {
				pObj = pBaseObj->makeCopy();
				pObj->decodeData(stream, kSkipRequireIO);
			}
		}
	}
	auto SList::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<SList>(
				TValue{mValue.mElemCode, DeepCopyTList(mValue.mList)});
		}
		return std::make_shared<SList>(*this);
	}
	auto SList::hasDefVal() const -> bool {
		return mValue.mList.size() == 0;
	}

}
