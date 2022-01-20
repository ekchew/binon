#ifndef BINON_NULLOBJ_HPP
#define BINON_NULLOBJ_HPP

#include "binonobj.hpp"

namespace binon {

	struct TNullObj: TStdCodecObj<TNullObj> {
		using TValue = void;
		static constexpr auto kTypeCode = kNullObjCode;
		static constexpr auto kClsName = std::string_view{"TNullObj"};
		constexpr auto operator== (TNullObj) const noexcept { return true; }
		constexpr auto hash() const noexcept -> std::size_t { return 0; }
		constexpr auto hasDefVal() const noexcept { return false; }
		constexpr void encodeData(TOStream&, bool requireIO = true)
			const noexcept {}
		constexpr void decodeData(CodeByte, TIStream&, bool requireIO = true)
			const noexcept {}
		constexpr void printArgs(std::ostream&) const noexcept {}
	};

	struct NullObj: BinONObj {
		using TValue = void;
		auto typeCode() const noexcept -> CodeByte final {return kNullObjCode;}
		auto getHash() const -> std::size_t override {return 0;}
		auto equals(const BinONObj& other) const -> bool override
			{ return other.typeCode() == kNullObjCode; }
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<NullObj>(); }
		auto clsName() const noexcept -> std::string override
			{ return "NullObj"; }
	};

}

namespace std {
	template<> struct hash<binon::NullObj> {
		constexpr auto operator () (const binon::NullObj&) const noexcept
			-> std::size_t { return 0; }
	};
}

#endif
