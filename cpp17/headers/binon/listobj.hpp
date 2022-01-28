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
	struct TVarObj;

	struct TCtnrType {};
	struct TListType: TCtnrType {};
	struct TListObj:
		TListType,
		TStdCtnr<TListObj, std::vector<TVarObj>>
	{
		static constexpr auto kTypeCode = kListObjCode;
		static constexpr auto kClsName = std::string_view{"TListObj"};
		using TStdCtnr<TListObj,TValue>::TStdCtnr;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TListObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TListObj&;
		void printArgs(std::ostream& stream) const;
	};
	struct TSList:
		TListType,
		TStdCtnr<TSList, std::vector<TVarObj>>
	{
		static constexpr auto kTypeCode = kSListCode;
		static constexpr auto kClsName = std::string_view{"TSList"};
		CodeByte mElemCode;
		TSList(std::any value, CodeByte elemCode = kNoObjCode);
		TSList(CodeByte elemCode = kNullObjCode);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TSList&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TSList&;
		void printArgs(std::ostream& stream) const;
	};

	//	See also list helper functions defined in typeconv.hpp.
}

#endif
