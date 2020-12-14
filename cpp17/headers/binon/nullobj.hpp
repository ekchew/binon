#ifndef BINON_NULLOBJ_HPP
#define BINON_NULLOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	
	class NullObj: public BinONObj {
	public:
		auto typeCode() const noexcept -> CodeByte final {return kNullObjCode;}
		auto getHash() const -> std::size_t override {return 0;}
		auto equals(const BinONObj& other) const -> bool override
			{ return other.typeCode() == kNullObjCode; }
		auto makeCopy() const -> TUPBinONObj override
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
