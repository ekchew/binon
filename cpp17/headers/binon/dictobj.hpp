#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "listobj.hpp"

#include <functional>
#include <optional>
#include <sstream>
#include <type_traits>
#include <unordered_map>

namespace binon {

	//	TDict is the value type of all dictionary objects (e.g. it is synomymous
	//	with DictObj::TValue). It is an unordered map where both key and value
	//	are BinON object types. The keys are presently restricted to
	//	non-container types that can be hashed.
	using TDict = std::unordered_map<
		BinONObj, BinONObj, std::hash<BinONObj>, std::equal_to<BinONObj>,
		BINON_ALLOCATOR<std::pair<const BinONObj, BinONObj>>
		>;

	//	DictType is a trivial base type of all dictionary objects. It is used by
	//	dictionary helper functions in typeconv.hpp to ascertain if a given type
	//	T is a DictObj, SKDict, or SDict.
	struct DictType: CtnrType{};
	struct DictBase: CtnrBase, CtnrType {
		using TValue = TDict;
		using CtnrBase::CtnrBase;
		auto operator == (const DictBase& rhs) const -> bool;
		auto operator != (const DictBase& rhs) const -> bool;
		auto hasDefVal() const -> bool;
		auto value() & -> TValue&;
		auto value() && -> TValue;
		auto value() const& -> const TValue&;
		auto size() const -> std::size_t;
	 protected:
	   auto calcHash(std::size_t seed) const -> std::size_t;
	};

	//	The 3 dictionary types: DictObj, SKDict, and SDict. See the note above
	//	StdCtnr regarding how values are managed in container types.
	struct DictObj: DictType, StdCtnr<DictObj, TDict> {
		using StdCtnr<DictObj, TDict>::StdCtnr;
		static constexpr auto kTypeCode = kDictObjCode;
		static constexpr auto kClsName = std::string_view{"DictObj"};
		auto hasDefVal() const -> bool;
		auto value() & -> TValue&;
		auto value() && -> TValue;
		auto value() const& -> const TValue&;
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
		auto hasDefVal() const -> bool;
		auto value() & -> TValue&;
		auto value() && -> TValue;
		auto value() const& -> const TValue&;
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
		auto hasDefVal() const -> bool;
		auto value() & -> TValue&;
		auto value() && -> TValue;
		auto value() const& -> const TValue&;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const SDict&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> SDict&;
		void printArgs(std::ostream& stream) const;
	};

	//	See also dict helper functions defined in dicthelpers.hpp.
}

#endif
