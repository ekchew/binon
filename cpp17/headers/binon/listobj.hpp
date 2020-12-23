#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include "binonobj.hpp"

namespace binon {

	auto DeepCopyTList(const TList& list) -> TList;
	void PrintTListRepr(const TList& list, std::ostream& stream);
	
	class ListObj:
		public BinONObj,
		public AccessContainer_mValue<ListObj,TList>
	{
	public:
		
		TValue mValue;
		
		ListObj(const TValue& v): mValue{v} {}
		ListObj(TValue&& v) noexcept: mValue{std::move(v)} {}
		ListObj(ListObj&& v) noexcept = default;
		ListObj(const ListObj& v) = default;
		ListObj() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final {return kListObjCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		
		//	encodeElems() is like encodeData() except that it does not encode
		//	the length of the list. It jumps straight to encoding the
		//	elements. These methods, then, are useful if you can already tell
		//	what the length is through some other means.
		void encodeElems(TOStream& stream, bool requireIO=true) const;
		void decodeElems(TIStream& stream, TValue::size_type count,
			bool requireIO=true);
		
		auto hasDefVal() const -> bool final;
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		void printRepr(std::ostream& stream) const override;
	};
	
	struct SListVal {
		CodeByte mElemCode = kIntObjCode;
		TList mList;
	};
	class SList:
		public BinONObj,
		public AccessContainer_mValue<SList,SListVal>
	{
	public:
		static constexpr bool kSkipAssertTypes = false;
		
		TValue mValue;
		
		SList(CodeByte elemCode) noexcept: mValue{elemCode} {}
		SList(const TValue& v): mValue{v} {}
		SList(TValue&& v) noexcept: mValue{std::move(v)} {}
		SList(const SList& v) = default;
		SList(SList&& v) noexcept = default;
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
		void printRepr(std::ostream& stream) const override;
	};

}

#endif
