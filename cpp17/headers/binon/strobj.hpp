#ifndef BINON_STROBJ_HPP
#define BINON_STROBJ_HPP

#include "binonobj.hpp"
#include "hystr.hpp"

namespace binon {
	struct StrObj: BinONObj, AccessContainer_mValue<StrObj,HyStr> {
		static void EncodeData(
			const TValue& v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		StrObj(const char* cStr): mValue{cStr} {}
		StrObj(const TValue& v): mValue{v} {}
		StrObj(TValue&& v) noexcept: mValue{std::move(v)} {}
		StrObj() noexcept = default;
		explicit operator bool() const noexcept override
			{ return mValue.size() != 0; }
		auto typeCode() const noexcept -> CodeByte final {return kStrObjCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto hash() const noexcept -> std::size_t
			{ return std::hash<TValue>{}(mValue); }
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kStrObjCode &&
					mValue == static_cast<const StrObj&>(other).mValue;
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<StrObj>(mValue); }
		auto clsName() const noexcept -> std::string override
			{ return "StrObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ stream << '"' << mValue << '"'; }
	};

}
namespace std {
	template<> struct hash<binon::StrObj> {
		auto operator () (const binon::StrObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
