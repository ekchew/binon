#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "listobj.hpp"

#include <functional>
#include <optional>
#include <sstream>
#include <type_traits>
#include <unordered_map>

namespace binon {
	using TDict = std::unordered_map<
		BinONObj, BinONObj, std::hash<BinONObj>, std::equal_to<BinONObj>,
		BINON_ALLOCATOR<std::pair<const BinONObj, BinONObj>>
		>;
	struct DictType{};
	struct DictObj: DictType, StdCtnr<DictObj, TDict> {
		using StdCtnr<DictObj, TDict>::StdCtnr;
		static constexpr auto kTypeCode = kDictObjCode;
		static constexpr auto kClsName = std::string_view{"DictObj"};
		auto encodeData(TOStream&, bool requireIO = true) const
			-> const DictObj&;
		auto decodeData(TIStream&, bool requireIO = true)
			-> DictObj&;
		void printArgs(std::ostream&) const;
	};
	struct SKDict: DictType, StdCtnr<SKDict,TDict> {
		static constexpr auto kTypeCode = kSKDictCode;
		static constexpr auto kClsName = std::string_view{"SKDict"};
		CodeByte mKeyCode;
		SKDict(std::any value, CodeByte keyCode = kNoObjCode);
		SKDict(CodeByte keyCode = kNoObjCode);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const SKDict&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> SKDict&;
		void printArgs(std::ostream& stream) const;
	};
	struct SDict: DictType, StdCtnr<SDict,TDict> {
		static constexpr auto kTypeCode = kSDictCode;
		static constexpr auto kClsName = std::string_view{"SDict"};
		CodeByte mKeyCode;
		CodeByte mValCode;
		SDict(std::any value,
			CodeByte keyCode = kNoObjCode, CodeByte valCode = kNoObjCode
			);
		SDict(CodeByte keyCode = kNoObjCode, CodeByte valCode = kNoObjCode);
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const SDict&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> SDict&;
		void printArgs(std::ostream& stream) const;
	};
}

#endif
