#include "binon/listobj.hpp"
#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"

#include <sstream>

namespace binon {

	ListObj::ListObj(const TValue& v):
		BinONObj{v.size() == 0}, mValue(v.size())
	{
		std::transform(
			v.begin(), v.end(), mValue.begin(),
			[](const TValue::value_type& p) { return p->makeCopy(); }
			);
	}
	auto ListObj::operator = (const ListObj& v) -> ListObj& {
		std::transform(
			v.mValue.begin(), v.mValue.end(), mValue.begin(),
			[](const TValue::value_type& p) { return p->makeCopy(); }
			);
		return *this;
	}
	void ListObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		UInt size{mValue.size()};
		size.encodeData(stream, false);
		for(auto&& pObj: mValue) {
			pObj->encode(stream, false);
		}
	}
	void ListObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt len;
		len.decodeData(stream, false);
		mValue.resize(len);
		for(auto&& pObj: mValue) {
			pObj = Decode(stream, false);
		}
	}
	
	SListValue::SListValue(const SListValue& v):
		mList(v.mList.size()), mTypeCode{v.mTypeCode}
	{
		std::transform(
			v.mList.begin(), v.mList.end(), mList.begin(),
			[](const TList::value_type& p) { return p->makeCopy(); }
			);
	}
	auto SListValue::operator = (const SListValue& v) -> SListValue& {
		std::transform(
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
		size.encodeData(stream, false);
		mValue.mTypeCode.write(stream, false);
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
					WriteWord(b, stream, false);
					b = 0x00_byte;
				}
			}
			if(i & 0x7) {
				b <<= 8 - i;
				WriteWord(b, stream, false);
			}
		}
		else {
			for(auto& pObj: mValue.mList) {
				checkType(pObj);
				pObj->encodeData(stream, false);
			}
		}
	}
	void SList::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UInt len;
		len.decodeData(stream, false);
		mValue.mList.resize(len);
		mValue.mTypeCode = CodeByte::Read(stream, false);
		if(mValue.mTypeCode.baseType() == kBoolObjCode.baseType()) {
			std::byte b = 0x00_byte;
			for(TList::size_type i = 0; i < mValue.mList.size(); ++i) {
				if((i & 0x7) == 0) {
					b = ReadWord<decltype(b)>(stream, false);
				}
				auto& pObj = mValue.mList[i];
				pObj = std::make_unique<BoolObj>((b & 0x80_byte) != 0x00_byte);
				b <<= 1;
			}
		}
		else {
			for(auto& pObj: mValue.mList) {
				pObj = FromTypeCode(mValue.mTypeCode);
				pObj->decodeData(stream, false);
			}
		}
	}

}
