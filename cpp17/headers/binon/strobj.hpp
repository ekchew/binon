#ifndef BINON_STROBJ_HPP
#define BINON_STROBJ_HPP

#include "binonobj.hpp"

#include <utility>
#include <string_view>

namespace binon {

	class StrObj: public BinONObj, public Read_mValue<StrObj,String> {
	public:
		TValue mValue;
		
		StrObj(const TValue& v): mValue{v} {}
		StrObj(TValue&& v) noexcept: mValue{std::forward<TValue>(v)} {}
		StrObj() noexcept = default;
		auto& operator = (StrObj obj)
			{ return std::swap(mValue, obj.mValue), *this; }
		auto typeCode() const noexcept -> CodeByte final {return kStrObjCode;}
		auto getStr() const -> const String& final {return mValue;}
		void encodeData(OStream& stream, bool requireIO=true) final;
		void decodeData(IStream& stream, bool requireIO=true) final;
		auto hash() const noexcept -> std::size_t
			{ return std::hash<String>{}(mValue); }
	};

}

namespace std {
	template<> struct hash<binon::StrObj> {
		constexpr auto operator () (const binon::StrObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
