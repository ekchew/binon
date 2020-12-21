#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include "binonobj.hpp"

namespace binon {

	auto DeepCopyTList(const TList& list) -> TList;
	
	class ListObj:
		public BinONObj,
		public AccessContainer_mValue<ListObj,TList>
	{
	public:
		TValue mValue;
		
		ListObj(const TValue& v);
		ListObj(TValue&& v) noexcept: mValue{std::move(v)} {}
		ListObj(const ListObj& v);
		ListObj(ListObj&& v) noexcept: ListObj{std::move(v.mValue)} {}
		ListObj() noexcept = default;
		auto operator = (const ListObj& v) -> ListObj&;
		auto& operator = (ListObj&& v) noexcept
			{ return mValue = std::move(v.mValue), *this; }
		auto typeCode() const noexcept -> CodeByte final;
		auto getList() const& -> const TValue& final;
		auto getList() && -> TValue&& final;
		void setList(TList v) final;
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		
		//	encodeElems() is like encodeData() except that it does not encode
		//	the length of the list. It jumps straight to encoding the
		//	elements. These methods, then, are useful if you can already tell
		//	what the length is through some other means.
		void encodeElems(TOStream& stream, bool requireIO=true) const;
		void decodeElems(TIStream& stream, TValue::size_type count,
			bool requireIO=true);
		
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto hasDefVal() const -> bool final;
	};
	
	struct SListValue {
		CodeByte mElemCode = kNullObjCode;
		TList mList;
	};
	class SList:
		public BinONObj,
		public AccessContainer_mValue<SList,SListValue>
	{
	public:
		static constexpr bool kSkipAssertTypes = false;
		
		TValue mValue;
		
		SList(const TValue& v);
		SList(TValue&& v) noexcept: mValue{std::move(v)} {}
		SList(const SList& v);
		SList(SList&& v) noexcept: SList{std::move(v.mValue)} {}
		SList(CodeByte elemCode) noexcept: mValue{elemCode} {}
		SList() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final;
		void assertElemTypes() const;
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		void encodeElems(TOStream& stream,
			bool requireIO=true, bool assertTypes=BINON_DEBUG) const;
		void decodeElems(TIStream& stream, TList::size_type count,
			bool requireIO=true);
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto hasDefVal() const -> bool final;
	};

}

#endif
