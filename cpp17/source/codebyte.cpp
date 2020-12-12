#include "binon/codebyte.hpp"
#include "binon/byteutil.hpp"

#include <sstream>

namespace binon {
	
	auto CodeByte::Read(TIStream& stream, bool requireIO) -> CodeByte {
		return FromInt(ReadWord<TStreamByte>(stream, requireIO));
	}
	void CodeByte::write(TOStream& stream, bool requireIO) const {
		WriteWord(toInt<TStreamByte>(), stream, requireIO);
	}
	
	auto BadCodeByte::WhatStr(CodeByte cb) -> std::string {
		std::ostringstream stream;
		auto hexArr = AsHex(static_cast<std::byte>(cb));
		stream << "invalid BinON code byte: 0x" << hexArr.data();
		return stream.str();
	}

}
