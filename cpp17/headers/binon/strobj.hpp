#ifndef BINON_STROBJ_HPP
#define BINON_STROBJ_HPP

#include "binonobj.hpp"

#include <string_view>

namespace binon {

	class StrObj:
		public BinONObj,
		public AccessContainer_mValue<StrObj,TString>
	{
	public:
		TValue mValue;
		
		StrObj(const TValue& v): mValue{v} {}
		StrObj(TValue&& v) noexcept: mValue{std::move(v)} {}
		StrObj(const StrObj& v): StrObj{v.mValue} {}
		StrObj(StrObj&& v) noexcept: StrObj{std::move(v.mValue)} {}
		StrObj() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final {return kStrObjCode;}
		auto getStr() const& -> const TString& final {return mValue;}
		auto getStr() && -> TString&& final {return std::move(mValue);}
		void setStr(TString v) final {std::swap(mValue, v);}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto hash() const noexcept -> std::size_t
			{ return std::hash<TString>{}(mValue); }
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kStrObjCode &&
					*this == static_cast<const StrObj&>(other);
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<StrObj>(mValue); }
		auto hasDefVal() const -> bool final { return mValue.size() == 0; }
	};

}

namespace std {
	template<> struct hash<binon::StrObj> {
		auto operator () (const binon::StrObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
