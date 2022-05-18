#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include <any>
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

	//	CtnrBase lies at the base of a class hierarchy that looks like this:
	//
	//		CtnrBase
	//			ListBase
	//				ListObj SList
	//			DictBase
	//				DictObj SKDict SDict
	//
	//	It stores the value used by all these classes in a std::any instance.
	//	This was done to get around a circular dependency problem. (For example,
	//	ListObj is built around a vector of BinONObj, but a BinONObj can itself
	//	be a ListObj. Each needs to know what the other is before it can be
	//	defined, leading to an impass. By using std::any, BinONObj no longer
	//	needs to know that a ListObj can contain them, thereby placating the
	//	compiler.)
	//
	//	Since any container object can be initialized with any data type besides
	//	the expected TList for list types or TDict for dictionary types, there
	//	can be no compile-time check that constructor is called with the right
	//	argument type. Instead, the value() methods perform this check at
	//	run-time and throw BadCtnrVal if there is a problem.
	struct CtnrBase{
		CtnrBase(const CtnrBase& ctnrBase);
		CtnrBase(CtnrBase&& ctnrBase) noexcept;
		CtnrBase() = default;
		auto operator= (const CtnrBase&) -> CtnrBase& = default;
		auto operator= (CtnrBase&&) noexcept -> CtnrBase& = default;
	 protected:
		std::any mValue;
		template<typename T> [[noreturn]] void castError();
	};

	//	ListBase implements a number of methods shared by ListObj and SList. In
	//	particular, it implements the value() methods used to extract a TList
	//	out of the CtnrBase's internal std::any member.
	struct ListBase: CtnrBase {
		using TValue = TList;
		using CtnrBase::CtnrBase;
		auto operator == (const ListBase& rhs) const -> bool;
		auto operator != (const ListBase& rhs) const -> bool;
		auto hasDefVal() const -> bool;
		auto value() & -> TValue&;
		auto value() && -> TValue;
		auto value() const& -> const TValue&;
		auto size() const -> std::size_t;
	 protected:
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

	//==== Template Implementation =============================================

	//---- CtnrBase ------------------------------------------------------------

	template<typename T> [[noreturn]] void CtnrBase::castError() {
		throw BadAnyCast::Make<T,BadCtnrVal>(
			mValue,
			"accessing BinON container value"
		);
	}
}

#endif
