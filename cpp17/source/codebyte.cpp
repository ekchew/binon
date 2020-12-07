#include "binon/codebyte.hpp"
#include "binon/byteutil.hpp"

#include <sstream>

namespace binon {
	
	auto CodeByte::Read(IStream& stream, bool requireIO) -> CodeByte {
		return FromInt(ReadWord<StreamByte>(stream, requireIO));
	}
	void CodeByte::write(OStream& stream, bool requireIO) {
		WriteWord(toInt<StreamByte>(), stream, requireIO);
	}
	
	auto BadCodeByte::WhatStr(CodeByte cb) -> std::string {
		std::ostringstream stream;
		auto hexArr = AsHex(static_cast<std::byte>(cb));
		stream << "invalid BinON code byte: 0x" << hexArr.data();
		return stream.str();
	}

}
