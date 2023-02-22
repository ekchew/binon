#include "binon/codebyte.hpp"

#include <sstream>

namespace binon {

	auto CodeByte::Read(TIStream& stream, bool requireIO) -> CodeByte {
		RequireIO rio{stream, requireIO};
		TStreamByte sb;
		stream.read(&sb, 1);
		return CodeByte::FromInt(sb);
	}

	void CodeByte::write(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto sb = this->asInt<TStreamByte>();
		stream.write(&sb, 1);
	}

	void CodeByte::printRepr(std::ostream& stream) const {
		switch(typeCode().asInt()) {
		 case kNullObjCode.asInt():
			stream << "kNullObjCode";
			break;
		 case kBoolObjCode.asInt():
			stream << "kBoolObjCode";
			break;
		 case kTrueObjCode.asInt():
			stream << "kTrueObjCode";
			break;
		 case kIntObjCode.asInt():
			stream << "kIntObjCode";
			break;
		 case kUIntCode.asInt():
			stream << "kUIntCode";
			break;
		 case kFloatObjCode.asInt():
			stream << "kFloatObjCode";
			break;
		 case kFloat32Code.asInt():
			stream << "kFloat32Code";
			break;
		 case kBufferObjCode.asInt():
			stream << "kBufferObjCode";
			break;
		 case kStrObjCode.asInt():
			stream << "kStrObjCode";
			break;
		 case kListObjCode.asInt():
			stream << "kListObjCode";
			break;
		 case kSListCode.asInt():
			stream << "kSListCode";
			break;
		 case kDictObjCode.asInt():
			stream << "kDictObjCode";
			break;
		 case kSKDictCode.asInt():
			stream << "kSKDictCode";
			break;
		 case kSDictCode.asInt():
			stream << "kSDictCode";
			break;
		 case kNoObjCode.asInt():
			stream << "kNoObjCode";
			break;
		 default:
			stream << "CodeByte{0x" << AsHex(_value) << "_byte}";
		}
	}

	auto operator << (std::ostream& stream, const CodeByte& cb)
		-> std::ostream&
	{
		return cb.printRepr(stream), stream;
	}

	auto BadCodeByte::WhatStr(CodeByte cb) -> std::string {
		std::ostringstream stream;
		auto hexArr = AsHex(static_cast<std::byte>(cb));
		stream << "invalid BinON code byte: 0x" << hexArr.data();
		return std::move(stream).str();
	}

}
