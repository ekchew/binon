#ifndef BINON_BOOLOBJ_HPP
#define BINON_BOOLOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	struct TBoolObj:
		TStdAcc<TBoolObj>,
		TStdEq<TBoolObj>,
		TStdHash<TBoolObj>,
		TStdHasDefVal<TBoolObj>
	{
		using TValue = bool;
		TValue mValue;
		static constexpr auto kTypeCode = kBoolObjCode;
		static constexpr auto kClsName = std::string_view{"BoolObj"};
		constexpr TBoolObj(TValue v = false) noexcept: mValue{v} {}
		void encode(TOStream& stream, bool requireIO = true) const;
		void decode(CodeByte cb, TIStream& stream, bool requireIO = true);
		void encodeData(TOStream&, bool requireIO = true) const {}
		void decodeData(TIStream&, bool requireIO = true) {}
		void printArgs(std::ostream& stream) const;
	};

	struct BoolObj: BinONObj, Access_mValue<BoolObj,bool> {
		static void EncodeData(TValue v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		BoolObj(TValue v=false) noexcept: mValue{v} {}
		explicit operator bool() const noexcept override
			{ return mValue; }
		auto typeCode() const noexcept -> CodeByte final {return kBoolObjCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kBoolObjCode &&
					mValue == static_cast<const BoolObj&>(other).mValue;
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<BoolObj>(mValue); }
		auto clsName() const noexcept -> std::string override
			{ return "BoolObj";}
 		void printArgsRepr(std::ostream& stream) const override
			{ stream << std::boolalpha << mValue; }
	};

	struct TrueObj: BinONObj, Access_mValue<TrueObj,bool> {
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
		auto clsName() const noexcept -> std::string override
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
