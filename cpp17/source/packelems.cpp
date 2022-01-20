#include "binon/packelems.hpp"
#include <sstream>

namespace binon {

	//---- PackElems -----------------------------------------------------------

	PackElems::PackElems(CodeByte elemCode, TOStream& stream, bool requireIO):
		RequireIO{stream, requireIO},
		mElemCode{elemCode},
		mByte{0x00_byte},
		mIndex{0}
	{
	}
	void PackElems::operator() (const VarObj& varObj) {
		if(varObj.typeCode() != mElemCode) {
			std::ostringstream oss;
			oss << "expected BinON element " << mIndex
				<< "to have type code ";
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
				WriteWord(mByte, stream(), kSkipRequireIO);
				mByte = 0x00_byte;
			}
		}
		else {
			std::visit(
				[this](const auto& obj) {
					obj.encodeData(stream(), kSkipRequireIO);
				},
				varObj
				);
			++mIndex;
		}
	}
	PackElems::~PackElems() {
		if(mElemCode == kBoolObjCode && mPStream) {
			auto n = mIndex & 0x7u;
			if(n != 0x0u) {
				WriteWord(mByte << (0x7u - n), stream(), kSkipRequireIO);
			}
		}
	}
	auto PackElems::stream() -> TOStream& {
		auto pStream = dynamic_cast<TOStream*>(mPStream);
		if(!pStream) {
			throw NullDeref{"PackElems has no valid stream pointer"};
		}
		return *pStream;
	}

	//---- UnpackElems ---------------------------------------------------------

	UnpackElems::UnpackElems(
		CodeByte elemCode, TIStream& stream, bool requireIO
		):
		RequireIO{stream, requireIO},
		mElemCode{elemCode},
		mByte{0x00_byte},
		mIndex{0}
	{
	}
	auto UnpackElems::operator() () -> VarObj {
		if(mElemCode == kBoolObjCode) {
			if((mIndex & 0x7u) == 0x0) {
				mByte = ReadWord<std::byte>(stream(), kSkipRequireIO);
			}
			TBoolObj boolObj{(mByte & 0x80_byte) != 0x00_byte};
			mByte <<= 1;
			++mIndex;
			return boolObj;
		}
		else {
			auto varObj{VarObj::FromTypeCode(mElemCode)};
			std::visit(
				[this](auto& obj) {
					obj.decodeData(stream(), kSkipRequireIO);
				},
				varObj
				);
			++mIndex;
			return varObj;
		}
	}
	auto UnpackElems::stream() -> TIStream& {
		auto pStream = dynamic_cast<TIStream*>(mPStream);
		if(!pStream) {
			throw NullDeref{"UnpackElems has no valid stream pointer"};
		}
		return *pStream;
	}
}
