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
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<NullObj>(); }
		auto hasDefVal() const -> bool final { return true; }
		void printRepr(std::ostream& stream) const override
			{ stream << "NullObj{" << mValue << '}'; }
	};

}

namespace std {
	template<> struct hash<binon::NullObj> {
		constexpr auto operator () (const binon::NullObj&) const noexcept
			-> std::size_t { return 0; }
	};
}

#endif
