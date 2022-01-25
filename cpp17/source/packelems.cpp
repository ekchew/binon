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
	void PackElems::operator() (const TVarObj& varObj, bool requireIO) {
		RequireIO rio{mStream, requireIO};
		if(varObj.typeCode() != mElemCode) {
			std::ostringstream oss;
			oss << "expected BinON element " << mIndex
				<< " to have type code ";
			mElemCode.printRepr(oss);
			oss << " rather than ";
			varObj.typeCode().printRepr(oss);
			oss << " (object: ";
			varObj.print(oss);
			oss << ")";
			throw TypeErr{oss.str()};
		}
		if(mElemCode == kBoolObjCode) {
			auto v = std::get<TBoolObj>(varObj).mValue;
			mByte <<= 1;
			if(v) {
				mByte |= 0x01_byte;
			}
			if((++mIndex & 0x7u) == 0x0u) {
				WriteWord(mByte, mStream, kSkipRequireIO);
				mByte = 0x00_byte;
			}
		}
		else {
			std::visit(
				[this](const auto& obj) {
					obj.encodeData(mStream, kSkipRequireIO);
				},
				varObj
				);
			++mIndex;
		}
	}
	PackElems::~PackElems() {
		if(mElemCode == kBoolObjCode) {
			auto n = mIndex & 0x7u;
			if(n != 0x0u) {
				WriteWord(mByte << (0x8u - n), mStream);
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
	auto UnpackElems::operator() (bool requireIO) -> TVarObj {
		RequireIO rio{mStream, requireIO};
		if(mElemCode == kBoolObjCode) {
			if((mIndex & 0x7u) == 0x0) {
				mByte = ReadWord<std::byte>(mStream, kSkipRequireIO);
			}
			TBoolObj boolObj{(mByte & 0x80_byte) != 0x00_byte};
			mByte <<= 1;
			++mIndex;
			return boolObj;
		}
		else {
			auto varObj{TVarObj::FromTypeCode(mElemCode)};
			std::visit(
				[this](auto& obj) {
					obj.decodeData(mStream, kSkipRequireIO);
				},
				varObj
				);
			++mIndex;
			return varObj;
		}
	}
}
