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
		
		StrObj(const TValue& v):
			BinONObj{v.size() == 0}, mValue{v} {}
		StrObj(TValue&& v) noexcept:
			BinONObj{v.size() == 0}, mValue{std::move(v)} {}
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
		auto makeCopy() const -> std::unique_ptr<BinONObj> override
			{ return std::make_unique<StrObj>(mValue); }
	};

}

namespace std {
	template<> struct hash<binon::StrObj> {
		constexpr auto operator () (const binon::StrObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
