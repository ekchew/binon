#ifndef BINON_PACKELEMS_HPP
#define BINON_PACKELEMS_HPP

#include "binonobj.hpp"

namespace binon {
	struct PackElems {
		PackElems(CodeByte elemCode, TOStream& stream);
		void operator() (const BinONObj& obj, bool requireIO = true);
		~PackElems();
	 private:
		CodeByte mElemCode;
		TOStream& mStream;
		std::byte mByte;
		std::size_t mIndex;
	};
	struct UnpackElems {
		UnpackElems(CodeByte elemCode, TIStream& stream);
		auto operator() (bool requireIO = true) -> BinONObj;
	 private:
		CodeByte mElemCode;
		TIStream& mStream;
		std::byte mByte;
		std::size_t mIndex;
	};
}

#endif
