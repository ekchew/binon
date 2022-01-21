#ifndef BINON_INTOBJ_HPP
#define BINON_INTOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	struct TIntObj:
		TStdAcc<TIntObj>,
		TStdEq<TIntObj>,
		TStdHash<TIntObj>,
		TStdHasDefVal<TIntObj>,
		TStdPrintArgs<TIntObj>,
		TStdCodec<TIntObj>
	{
		using TValue = std::int64_t;
		static constexpr auto kTypeCode = kIntObjCode;
		static constexpr auto kClsName = std::string_view{"TIntObj"};
		TValue mValue;
		constexpr TIntObj(TValue v = 0) noexcept: mValue{v} {}
		void encodeData(TOStream& stream, bool requireIO = true) const;
		void decodeData(TIStream& stream, bool requireIO = true);
	};
	struct TUIntObj:
		TStdAcc<TUIntObj>,
		TStdEq<TUIntObj>,
		TStdHash<TUIntObj>,
		TStdHasDefVal<TUIntObj>,
		TStdPrintArgs<TUIntObj>,
		TStdCodec<TUIntObj>
	{
		using TValue = std::uint64_t;
		static constexpr auto kTypeCode = kUIntCode;
		static constexpr auto kClsName = std::string_view{"TUIntObj"};
		TValue mValue;
		constexpr TUIntObj(TValue v = 0) noexcept: mValue{v} {}
		void encodeData(TOStream& stream, bool requireIO = true) const;
		void decodeData(TIStream& stream, bool requireIO = true);
	};

	struct IntRangeError: std::range_error {
		IntRangeError();
	};

	struct IntObj: BinONObj, Access_mValue<IntObj,std::int64_t> {
		static void EncodeData(TValue v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		IntObj(TValue v=0) noexcept: mValue{v} {}
		explicit operator bool() const noexcept override
			{ return mValue != 0; }
		auto typeCode() const noexcept -> CodeByte final {return kIntObjCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kIntObjCode &&
					mValue == static_cast<const IntObj&>(other).mValue;
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<IntObj>(mValue); }
		auto clsName() const noexcept -> std::string override
			{ return "IntObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ stream << mValue; }
	};

	struct UIntObj: BinONObj, Access_mValue<UIntObj,std::uint64_t> {
		static void EncodeData(
			TValue v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		UIntObj(TValue v=0) noexcept: mValue{v} {}
		explicit operator bool() const noexcept override
			{ return mValue != 0; }
		auto typeCode() const noexcept -> CodeByte final {return kUIntCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kUIntCode &&
					mValue == static_cast<const UIntObj&>(other).mValue;
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<UIntObj>(mValue); }
		auto clsName() const noexcept -> std::string override
			{ return "UIntObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ stream << mValue; }
	};

	namespace types {
		using UInt = UIntObj;
	}
}

namespace std {
	template<> struct hash<binon::IntObj> {
		constexpr auto operator () (const binon::IntObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
	template<> struct hash<binon::UIntObj> {
		 constexpr auto operator () (const binon::UIntObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
