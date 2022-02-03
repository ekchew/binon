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
	struct ListType: CtnrType{};
	struct ListObj: ListType, StdCtnr<ListObj,TList> {
		static constexpr auto kTypeCode = kListObjCode;
		static constexpr auto kClsName = std::string_view{"ListObj"};
		using StdCtnr<ListObj,TList>::StdCtnr;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const ListObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> ListObj&;
		void printArgs(std::ostream& stream) const;
	};
	struct SList: ListType, StdCtnr<SList,TList> {
		static constexpr auto kTypeCode = kSListCode;
		static constexpr auto kClsName = std::string_view{"SList"};
		CodeByte mElemCode;
		SList(std::any value, CodeByte elemCode = kNoObjCode);
		SList(CodeByte elemCode = kNullObjCode);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const SList&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> SList&;
		void printArgs(std::ostream& stream) const;
	};

	//	See also list helper functions defined in listhelpers.hpp.
}

#endif
