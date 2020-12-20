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

	ListObj::ListObj(const TValue& v):
		BinONObj{v.size() == 0}, mValue(v.size())
	{
		std::transform(BINON_PAR_UNSEQ
			v.begin(), v.end(), mValue.begin(),
			[](const TValue::value_type& p) { return p->makeCopy(); }
			);
	}
	auto ListObj::operator = (const ListObj& v) -> ListObj& {
		return *this = ListObj{v};
	}
	void ListObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		UInt size{mValue.size()};
		size.encodeData(stream, kSkipRequireIO);
		for(auto&& pObj: mValue) {
			pObj->encode(stream, kSkipRequireIO);
		}
	}
	void ListObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt len;
		len.decodeData(stream, kSkipRequireIO);
		mValue.resize(len);
		for(auto&& pObj: mValue) {
			pObj = Decode(stream, kSkipRequireIO);
		}
	}
	
	SListValue::SListValue(const SListValue& v):
		mList(v.mList.size()), mTypeCode{v.mTypeCode}
	{
		std::transform(BINON_PAR_UNSEQ
			v.mList.begin(), v.mList.end(), mList.begin(),
			[](const TList::value_type& p) { return p->makeCopy(); }
			);
	}
	auto SListValue::operator = (const SListValue& v) -> SListValue& {
		std::transform(BINON_PAR_UNSEQ
			v.mList.begin(), v.mList.end(), mList.begin(),
			[](const TList::value_type& p) { return p->makeCopy(); }
			);
		mTypeCode = v.mTypeCode;
		return *this;
	}
	auto& SListValue::operator = (SListValue&& v) noexcept {
		mList = std::move(v.mList);
		mTypeCode = v.mTypeCode;
		return *this;
	}
	
	void SList::encodeData(TOStream& stream, bool requireIO) const {
		auto checkType = [this](const TList::value_type& pObj) {
			if(pObj->typeCode() != mValue.mTypeCode) {
				std::ostringstream oss;
				oss << "SList expected type code " << AsHex(mValue.mTypeCode)
					<< " but encountered " << AsHex(pObj->typeCode());
				throw TypeErr{oss.str()};
			}
		};
		RequireIO rio{stream, requireIO};
		UInt size{mValue.mList.size()};
		size.encodeData(stream, kSkipRequireIO);
		mValue.mTypeCode.write(stream, kSkipRequireIO);
		if(mValue.mTypeCode.baseType() == kBoolObjCode.baseType()) {
			std::byte b = 0x00_byte;
			TList::size_type i = 0;
			for(; i < mValue.mList.size(); ++i) {
				auto& pObj = mValue.mList[i];
				b <<= 1;
				if(pObj->getBool()) {
					b |= 0x01_byte;
				}
				if((i & 0x7) == 0x7) {
					WriteWord(b, stream, kSkipRequireIO);
					b = 0x00_byte;
				}
			}
			if(i & 0x7) {
				b <<= 8 - i;
				WriteWord(b, stream, kSkipRequireIO);
			}
		}
		else {
			for(auto& pObj: mValue.mList) {
				checkType(pObj);
				pObj->encodeData(stream, kSkipRequireIO);
			}
		}
	}
	void SList::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt len;
		len.decodeData(stream, kSkipRequireIO);
		mValue.mList.resize(len);
		mValue.mTypeCode = CodeByte::Read(stream, kSkipRequireIO);
		if(mValue.mTypeCode.baseType() == kBoolObjCode.baseType()) {
			std::byte b = 0x00_byte;
			for(TList::size_type i = 0; i < mValue.mList.size(); ++i) {
				if((i & 0x7) == 0) {
					b = ReadWord<decltype(b)>(stream, kSkipRequireIO);
				}
				auto& pObj = mValue.mList[i];
				pObj = std::make_unique<BoolObj>((b & 0x80_byte) != 0x00_byte);
				b <<= 1;
			}
		}
		else {
			for(auto& pObj: mValue.mList) {
				pObj = FromTypeCode(mValue.mTypeCode);
				pObj->decodeData(stream, kSkipRequireIO);
			}
		}
	}

}
