#ifndef BINON_BOOLOBJ_HPP
#define BINON_BOOLOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	
	class BoolObj: public BinONObj, public Access_mValue<BoolObj,bool> {
	public:
		TValue mValue;
		
		BoolObj(TValue v=false) noexcept: BinONObj{!v}, mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kBoolObjCode;}
		auto getBool() const -> TValue final {return mValue;}
		void setBool(TValue v) final {mValue = v;}
		void encodeData(TOStream& stream, TValue requireIO=true) const final;
		void decodeData(TIStream& stream, TValue requireIO=true) final;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kBoolObjCode &&
					*this == static_cast<const BoolObj&>(other);
			}
		auto makeCopy() const -> TUPBinONObj override
			{ return std::make_unique<BoolObj>(mValue); }
	};
	
	class TrueObj: public BinONObj, public Access_mValue<TrueObj,bool> {
	public:
		struct Value {
			constexpr operator TValue() const noexcept {return true;}
		};
		Value mValue;
		
		auto typeCode() const noexcept -> CodeByte final
			{ return kTrueObjCode; }
		auto getBool() const -> TValue final {return true;}
		void setBool(TValue value) final {if(!value) typeErr();}
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override
			{ return other.typeCode() == kTrueObjCode; }
		auto makeCopy() const -> TUPBinONObj override
			{ return std::make_unique<TrueObj>(); }
	};

}

namespace std {
	template<> struct hash<binon::BoolObj> {
		constexpr auto operator () (const binon::BoolObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
	template<> struct hash<binon::TrueObj> {
		constexpr auto operator () (const binon::TrueObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
