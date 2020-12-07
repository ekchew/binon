#ifndef BINON_NULLOBJ_HPP
#define BINON_NULLOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	
	class NullObj: public BinONObj {
	public:
		auto typeCode() const noexcept -> CodeByte final {return kNullObjCode;}
	};

}

#endif
