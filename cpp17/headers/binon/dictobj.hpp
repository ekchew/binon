#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "listobj.hpp"

#include <functional>
#include <optional>
#include <sstream>
#include <type_traits>
#include <unordered_map>

namespace binon {
	struct SKDict;
	struct SDict;

	//	TDict is the value type of all dictionary objects (e.g. it is synomymous
	//	with DictObj::TValue). It is an unordered map where both key and value
	//	are BinON object types. The keys are presently restricted to
	//	non-container types that can be hashed.
	using TDict = std::unordered_map<
		BinONObj, BinONObj, std::hash<BinONObj>, std::equal_to<BinONObj>,
		BINON_ALLOCATOR<std::pair<const BinONObj, BinONObj>>
		>;

	//	DictBase is very much the dictionary counterpart to ListBase. Its
	//	value() methods return a TDict rather than a TList, of course.
	struct DictBase: CtnrBase {
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

	struct DictObj: DictBase, StdCodec<DictObj> {
		static constexpr auto kTypeCode = kDictObjCode;
		static constexpr auto kClsName = std::string_view{"DictObj"};
		explicit DictObj(const SKDict& obj);
		explicit DictObj(const SDict& obj);
		using DictBase::DictBase;
		auto encodeData(TOStream&, bool requireIO = true) const
			-> const DictObj&;
		auto decodeData(TIStream&, bool requireIO = true)
			-> DictObj&;
		auto hash() const -> std::size_t;
		void printArgs(std::ostream&) const;
	};
	struct SKDict: DictBase, StdCodec<SKDict> {
		static constexpr auto kTypeCode = kSKDictCode;
		static constexpr auto kClsName = std::string_view{"SKDict"};
		CodeByte mKeyCode;
		SKDict(const std::any& value, CodeByte keyCode);
		SKDict(std::any&& value, CodeByte keyCode) noexcept;
		SKDict(CodeByte keyCode = kNoObjCode) noexcept;
		explicit SKDict(const SDict& obj);
		using DictBase::DictBase;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const SKDict&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> SKDict&;
		auto hash() const -> std::size_t;
		void printArgs(std::ostream& stream) const;
	};
	struct SDict: DictBase, StdCodec<SDict> {
		static constexpr auto kTypeCode = kSDictCode;
		static constexpr auto kClsName = std::string_view{"SDict"};
		CodeByte mKeyCode;
		CodeByte mValCode;
		SDict(const std::any& value,
			CodeByte keyCode, CodeByte valCode
			);
		SDict(std::any&& value,
			CodeByte keyCode, CodeByte valCode
			) noexcept;
		SDict(
			CodeByte keyCode = kNoObjCode, CodeByte valCode = kNoObjCode
			) noexcept;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const SDict&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> SDict&;
		auto hash() const -> std::size_t;
		void printArgs(std::ostream& stream) const;
	};

	//	See also dict helper functions defined in dicthelpers.hpp.
}

#endif
