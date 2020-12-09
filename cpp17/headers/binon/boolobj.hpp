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
		void encodeData(OStream& stream, TValue requireIO=true) final;
		void decodeData(IStream& stream, TValue requireIO=true) final;
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
