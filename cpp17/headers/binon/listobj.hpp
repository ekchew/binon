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
			BinONObj{v.size() == 0}, mValue{std::move(v)} {}
		ListObj(const ListObj& v): ListObj{v.mValue} {}
		ListObj(ListObj&& v) noexcept: ListObj{std::move(v.mValue)} {}
		ListObj() noexcept = default;
		auto operator = (const ListObj& v) -> ListObj&;
		auto& operator = (ListObj&& v) noexcept
			{ return mValue = std::move(v.mValue), *this; }
		auto typeCode() const noexcept -> CodeByte final {return kListObjCode;}
		auto getList() const& -> const TValue& final {return mValue;}
		auto getList() && -> TValue&& final {return std::move(mValue);}
		void setList(TList v) final {std::swap(mValue, v);}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto makeCopy() const -> TUPBinONObj override
			{ return std::make_unique<ListObj>(mValue); }
	};
	
	struct SListValue {
		TList mList;
		CodeByte mTypeCode;
		
		SListValue(const SListValue& v);
		SListValue(SListValue&& v) noexcept:
			mList{std::move(v.mList)}, mTypeCode{v.mTypeCode} {}
		SListValue(CodeByte elemCode=kIntObjCode) noexcept:
			mTypeCode{elemCode} {}
		auto operator = (const SListValue& v) -> SListValue&;
		auto& operator = (SListValue&& v) noexcept;
	};
	class SList:
		public BinONObj,
		public AccessContainer_mValue<ListObj,SListValue>
	{
	public:
		TValue mValue;
		
		SList(const TValue& v):
			BinONObj{v.mList.size() == 0}, mValue{v} {}
		SList(TValue&& v) noexcept:
			BinONObj{v.mList.size() == 0}, mValue{std::move(v)} {}
		SList(const SList& v): SList{v.mValue} {}
		SList(SList&& v) noexcept: SList{std::move(v.mValue)} {}
		SList(CodeByte elemCode=kNullObjCode) noexcept: mValue{elemCode} {}
		auto typeCode() const noexcept -> CodeByte final {return kSListCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto makeCopy() const -> TUPBinONObj override
			{ return std::make_unique<SList>(mValue); }
	};

}

#endif
