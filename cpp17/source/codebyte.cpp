#include "binon/codebyte.hpp"

#include <sstream>

namespace binon {

	auto CodeByte::Read(TIStream& stream, bool requireIO) -> CodeByte {
		return ByteUnpack<std::byte>(stream, requireIO);
	}
	void CodeByte::write(TOStream& stream, bool requireIO) const {
		BytePack(mValue, stream, requireIO);
	}
	void CodeByte::printRepr(std::ostream& stream) const {
		switch(asUInt()) {
		 case kNullObjCode.asUInt():
			stream << "kNullObjCode";
			break;
		 case kBoolObjCode.asUInt():
			stream << "kBoolObjCode";
			break;
		 case kTrueObjCode.asUInt():
			stream << "kTrueObjCode";
			break;
		 case kIntObjCode.asUInt():
			stream << "kIntObjCode";
			break;
		 case kUIntCode.asUInt():
			stream << "kUIntCode";
			break;
		 case kFloatObjCode.asUInt():
			stream << "kFloatObjCode";
			break;
		 case kFloat32Code.asUInt():
			stream << "kFloat32Code";
			break;
		 case kBufferObjCode.asUInt():
			stream << "kBufferObjCode";
			break;
		 case kStrObjCode.asUInt():
			stream << "kStrObjCode";
			break;
		 case kListObjCode.asUInt():
			stream << "kListObjCode";
			break;
		 case kSListCode.asUInt():
			stream << "kSListCode";
			break;
		 case kDictObjCode.asUInt():
			stream << "kDictObjCode";
			break;
		 case kSKDictCode.asUInt():
			stream << "kSKDictCode";
			break;
		 case kSDictCode.asUInt():
			stream << "kSDictCode";
			break;
		 case kNoObjCode.asUInt():
			stream << "kNoObjCode";
			break;
		 default:
			stream << "CodeByte{";
			PrintByte(mValue, stream);
			stream << '}';
		}
	}
	auto operator<< (std::ostream& stream, const CodeByte& cb) -> std::ostream&
	{
		cb.printRepr(stream);
		return stream;
	}

	auto BadCodeByte::WhatStr(CodeByte cb) -> std::string {
		std::ostringstream stream;
		auto hexArr = AsHex(static_cast<std::byte>(cb));
		stream << "invalid BinON code byte: 0x" << hexArr.data();
		return std::move(stream).str();
	}

}
