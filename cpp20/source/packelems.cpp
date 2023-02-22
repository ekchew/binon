#include "binon/packelems.hpp"
#include <sstream>

namespace binon {

	//---- PackElems -----------------------------------------------------------

	PackElems::PackElems(CodeByte elemCode, TOStream& stream):
		mElemCode{elemCode},
		mStream{stream},
		mByte{0x00_byte},
		mIndex{0}
	{
	}
	void PackElems::operator() (const BinONObj& varObj, bool requireIO) {
		RequireIO rio{mStream, requireIO};
		if(varObj.typeCode() != mElemCode) {
			std::ostringstream oss;
			oss << "expected BinON container element " << mIndex
				<< " to have type code ";
			mElemCode.printRepr(oss);
			oss << " rather than ";
			varObj.typeCode().printRepr(oss);
			oss << " (object: ";
			varObj.print(oss);
			oss << ")";
			throw BadElemType{oss.str()};
		}
		if(mElemCode == kBoolObjCode) {
			auto v = std::get<BoolObj>(varObj).mValue;
			mByte <<= 1;
			if(v) {
				mByte |= 0x01_byte;
			}
			if((++mIndex & 0x7u) == 0x0u) {
				BytePack<kSkipRequireIO>(mStream, mByte);
				mByte = 0x00_byte;
			}
		}
		else {
			varObj.encodeData(mStream, kSkipRequireIO);
			++mIndex;
		}
	}
	PackElems::~PackElems() {
		if(mElemCode == kBoolObjCode) {
			auto n = mIndex & 0x7u;
			if(n != 0x0u) {
				BytePack(mStream, mByte << (0x8u - n));
			}
		}
	}

	//---- UnpackElems ---------------------------------------------------------

	UnpackElems::UnpackElems(CodeByte elemCode, TIStream& stream):
		mElemCode{elemCode},
		mStream{stream},
		mByte{0x00_byte},
		mIndex{0}
	{
	}
	auto UnpackElems::operator() (bool requireIO) -> BinONObj {
		RequireIO rio{mStream, requireIO};
		if(mElemCode == kBoolObjCode) {
			if((mIndex & 0x7u) == 0x0) {
				mByte = ByteUnpack<std::byte, kSkipRequireIO>(mStream);
			}
			BoolObj boolObj{(mByte & 0x80_byte) != 0x00_byte};
			mByte <<= 1;
			++mIndex;
			return boolObj;
		}
		else {
			auto varObj{BinONObj::FromTypeCode(mElemCode)};
			varObj.decodeData(mStream, kSkipRequireIO);
			++mIndex;
			return varObj;
		}
	}
}
