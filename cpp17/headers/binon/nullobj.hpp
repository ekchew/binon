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

namespace std {
	template<> struct hash<binon::NullObj> {
		constexpr auto operator () (const binon::NullObj&) const noexcept
			-> std::size_t { return 0; }
	};
}

#endif
