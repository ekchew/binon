#ifndef BINON_NULLOBJ_HPP
#define BINON_NULLOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	
	class NullObj: public BinONObj {
	public:
		auto typeCode() const noexcept -> CodeByte final {return kNullObjCode;}
		auto makeCopy() const -> std::unique_ptr<BinONObj> override
			{ return std::make_unique<NullObj>(); }
	};

}

#endif
