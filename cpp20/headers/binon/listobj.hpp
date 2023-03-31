#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include <functional>
#include <initializer_list>
#include <optional>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include "mixins.hpp"

namespace binon {
	struct BinONObj;
	struct SList;

	using TList = std::vector<BinONObj,BINON_ALLOCATOR<BinONObj>>;

	//	ListBase implements a number of methods shared by ListObj and SList.
	struct ListBase {
		using TValue = TList;
		ListBase(const ListBase&) = default;
		ListBase(ListBase&&) noexcept = default;
		ListBase() = default;
		auto operator = (const ListBase&) -> ListBase& = default;
		auto operator = (ListBase&&) noexcept -> ListBase& = default;
		auto operator == (const ListBase& rhs) const -> bool;
		auto operator != (const ListBase& rhs) const -> bool;
		auto value() & -> TValue& { return mValue; }
		auto value() && -> TValue { return std::move(mValue); }
		auto value() const& -> const TValue& { return mValue; }
		auto size() const -> std::size_t;
		auto hasDefVal() const -> bool { return size() == 0; }

	 protected:
		TList mValue;
		auto calcHash(std::size_t seed) const -> std::size_t;
	};

	struct ListObj: ListBase, StdCodec<ListObj>  {
		static constexpr auto kTypeCode = kListObjCode;
		static constexpr auto kClsName = std::string_view{"ListObj"};
		explicit ListObj(const SList& obj);
		ListObj(const TList& list);
		ListObj(TList&& list) noexcept;
		using ListBase::ListBase;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const ListObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> ListObj&;
		auto hash() const -> std::size_t;
		void printArgs(std::ostream& stream) const;
	};

	struct SList: ListBase, StdCodec<SList> {
		static constexpr auto kTypeCode = kSListCode;
		static constexpr auto kClsName = std::string_view{"SList"};
		CodeByte mElemCode;
		SList(const TList& list, CodeByte elemCode);
		SList(TList&& list, CodeByte elemCode) noexcept;
		SList(CodeByte elemCode = kNoObjCode);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const SList&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> SList&;
		auto hash() const -> std::size_t;
		void printArgs(std::ostream& stream) const;
	};

	//	See also list helper functions defined in listhelpers.hpp.
}

#endif
