#ifndef BINON_NULLOBJ_HPP
#define BINON_NULLOBJ_HPP

#include "binonobj.hpp"
#include <cstddef>

namespace binon {

	struct TNullObj: TStdCodec<TNullObj> {
		using TValue = std::nullptr_t;
		TValue mValue = nullptr;
		static constexpr auto kTypeCode = kNullObjCode;
		static constexpr auto kClsName = std::string_view{"TNullObj"};
		auto value() const { return mValue; }
		auto& value() { return mValue; }
		constexpr auto operator== (TNullObj) const noexcept { return true; }
		constexpr auto operator!= (TNullObj) const noexcept { return false; }
		auto hash() const noexcept -> std::size_t {
				return std::hash<CodeByte>{}(kTypeCode);
			}
		constexpr auto hasDefVal() const noexcept { return false; }
		constexpr void encodeData(TOStream&, bool requireIO = true)
			const noexcept {}
		constexpr void decodeData(TIStream&, bool requireIO = true)
			noexcept {}
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
