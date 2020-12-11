#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include "binonobj.hpp"

#include <type_traits>

namespace binon {

	class ListObj:
		public BinONObj,
		public AccessContainer_mValue<ListObj,TList>
	{
	public:
		TValue mValue;
		
		ListObj(const TValue& v);
		ListObj(TValue&& v) noexcept:
			BinONObj{v.size() == 0}, mValue{std::move(v)} {}
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
	
	template<typename Elem> class SList:
		public BinONObj,
		public AccessContainer_mValue<SList<Obj>,TVector<Obj>>
	{
		static_assert(
			std::is_base_of_v<BinONObj, Elem>,
			"SList element type must be a subclass of BinONObj"
			);
	public:
		TValue mValue;
		
		SList(const TValue& v): BinONObj{v.size() == 0}, mValue{v} {}
		SList(TValue&& v) noexcept:
			BinONObj{v.size() == 0}, SList{std::move(v)} {}
		SList(const SList& v): SList{v.mValue} {}
		SList(SList&& v) noexcept: SList{std::move(v.mValue)} {}
		SList() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final {return kSListCode;}
	};

}

#endif
