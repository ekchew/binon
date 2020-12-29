#ifndef BINON_BOOLOBJ_HPP
#define BINON_BOOLOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	
	class BoolObj: public BinONObj, public Access_mValue<BoolObj,bool> {
	public:
		TValue mValue;
		
		BoolObj(TValue v=false) noexcept: mValue{v} {}
		explicit operator bool() const noexcept override
			{ return mValue; }
		auto typeCode() const noexcept -> CodeByte final {return kBoolObjCode;}
		void encodeData(TOStream& stream, TValue requireIO=true) const final;
		void decodeData(TIStream& stream, TValue requireIO=true) final;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kBoolObjCode &&
					*this == static_cast<const BoolObj&>(other);
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<BoolObj>(mValue); }
		auto clsName() const noexcept -> const char* override
			{ return "BoolObj";}
 		void printArgsRepr(std::ostream& stream) const override
			{ stream << std::boolalpha << mValue; }
	};
	
	class TrueObj: public BinONObj, public Access_mValue<TrueObj,bool> {
	public:
		struct Value {
			constexpr operator TValue() const noexcept {return true;}
		};
		Value mValue;
		
		explicit operator bool() const noexcept override
			{ return true; }
		auto typeCode() const noexcept -> CodeByte final
			{ return kTrueObjCode; }
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override
			{ return other.typeCode() == kTrueObjCode; }
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<TrueObj>(); }
		auto clsName() const noexcept -> const char* override
			{ return "TrueObj"; }
		void printRepr(std::ostream& stream) const override
			{ stream << "true"; }
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
