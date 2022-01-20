#ifndef BINON_PACKELEMS_HPP
#define BINON_PACKELEMS_HPP

#include "varobj.hpp"

namespace binon {
	struct PackElems: RequireIO {
		PackElems(CodeByte elemCode, TOStream& stream, bool requireIO = true);
		void operator() (const VarObj& obj);
		~PackElems();
	 private:
		CodeByte mElemCode;
		std::byte mByte;
		std::size_t mIndex;
		auto stream() -> TOStream&;
	};
	struct UnpackElems: RequireIO {
		UnpackElems(CodeByte elemCode, TIStream& stream, bool requireIO = true);
		auto operator() () -> VarObj;
	 private:
		CodeByte mElemCode;
		std::byte mByte;
		std::size_t mIndex;
		auto stream() -> TIStream&;
	};
}

#endif
