#ifndef BINON_PACKELEMS_HPP
#define BINON_PACKELEMS_HPP

#include "varobj.hpp"

namespace binon {
	struct PackElems {
		PackElems(CodeByte elemCode, TOStream& stream);
		void operator() (const TVarObj& obj, bool requireIO = true);
		~PackElems();
	 private:
		CodeByte mElemCode;
		TOStream& mStream;
		std::byte mByte;
		std::size_t mIndex;
	};
	struct UnpackElems {
		UnpackElems(CodeByte elemCode, TIStream& stream);
		auto operator() (bool requireIO = true) -> TVarObj;
	 private:
		CodeByte mElemCode;
		TIStream& mStream;
		std::byte mByte;
		std::size_t mIndex;
	};
}

#endif
