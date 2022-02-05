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

	using TList = std::vector<BinONObj,BINON_ALLOCATOR<BinONObj>>;

	struct CtnrType{};
	struct CtnrBase{
		CtnrBase(const std::any& ctnr);
		CtnrBase(std::any&& ctnr) noexcept;
		CtnrBase() = default;
	 protected:
		std::any mValue;
		[[noreturn]] static void CastError();
	};
	struct ListBase: CtnrBase, CtnrType {
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
		SList(std::any value, CodeByte elemCode = kNoObjCode);
		SList(CodeByte elemCode = kNullObjCode);
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
