#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include "binonobj.hpp"

namespace binon {

	class ListObj:
		public BinONObj,
		public AccessContainer_mValue<ListObj,TList>
	{
	public:
		TValue mValue;
		
		ListObj(const TValue& v);
		ListObj(TValue&& v) noexcept:
			BinONObj{v.size() == 0}, mValue{std::forward<TValue>(v)} {}
		ListObj(const ListObj& v): ListObj{v.mValue} {}
		ListObj(ListObj&& v) noexcept: ListObj{std::move(v.mValue)} {}
		ListObj() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final {return kListObjCode;}
		auto getList() const& -> const TValue& final {return mValue;}
		auto getList() && -> TValue&& final {return std::move(mValue);}
		void setList(TList v) final {std::swap(mValue, v);}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto makeCopy() const -> std::unique_ptr<BinONObj> override
			{ return std::make_unique<ListObj>(mValue); }
	};

}

#endif
