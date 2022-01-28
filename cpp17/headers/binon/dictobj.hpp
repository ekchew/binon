#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "listobj.hpp"

#include <functional>
#include <optional>
#include <sstream>
#include <type_traits>
#include <unordered_map>

namespace binon {
	struct TDictType: TCtnrType {};
	struct TDictObj:
		TDictType,
		TStdCtnr<TDictObj, std::unordered_map<TVarObj,TVarObj>>
	{
		using TStdCtnr<TDictObj, std::unordered_map<TVarObj,TVarObj>>::TStdCtnr;
		static constexpr auto kTypeCode = kDictObjCode;
		static constexpr auto kClsName = std::string_view{"TDictObj"};
		auto encodeData(TOStream&, bool requireIO = true) const
			-> const TDictObj&;
		auto decodeData(TIStream&, bool requireIO = true)
			-> TDictObj&;
		void printArgs(std::ostream&) const;
	};
	struct TSKDict:
		TDictType,
		TStdCtnr<TSKDict, std::unordered_map<TVarObj,TVarObj>>
	{
		static constexpr auto kTypeCode = kSKDictCode;
		static constexpr auto kClsName = std::string_view{"TSKDict"};
		CodeByte mKeyCode;
		TSKDict(std::any value, CodeByte keyCode = kNoObjCode);
		TSKDict(CodeByte keyCode = kNoObjCode);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TSKDict&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TSKDict&;
		void printArgs(std::ostream& stream) const;
	};
	struct TSDict:
		TDictType,
		TStdCtnr<TSDict, std::unordered_map<TVarObj,TVarObj>>
	{
		static constexpr auto kTypeCode = kSDictCode;
		static constexpr auto kClsName = std::string_view{"TSDict"};
		CodeByte mKeyCode;
		CodeByte mValCode;
		TSDict(std::any value,
			CodeByte keyCode = kNoObjCode, CodeByte valCode = kNoObjCode
			);
		TSDict(CodeByte keyCode = kNoObjCode, CodeByte valCode = kNoObjCode);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TSDict&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TSDict&;
		void printArgs(std::ostream& stream) const;
	};
}

#endif
