#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "listobj.hpp"

#include <any>
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
	struct DictBase {
		using TValue = TDict;
		DictBase(const DictBase&) = default;
		DictBase(DictBase&&) noexcept = default;
		DictBase() = default;
		auto operator= (const DictBase&) -> DictBase& = default;
		auto operator= (DictBase&&) noexcept -> DictBase& = default;
		auto operator == (const DictBase& rhs) const -> bool;
		auto operator != (const DictBase& rhs) const -> bool;
		auto hasDefVal() const -> bool;
		auto value() & -> TValue&;
		auto value() && -> TValue;
		auto value() const& -> const TValue&;
		auto size() const -> std::size_t;
	 protected:
		std::any mValue;
		auto calcHash(std::size_t seed) const -> std::size_t;
		template<typename T> [[noreturn]] void castError();
	};

	struct DictObj: DictBase, StdCodec<DictObj> {
		static constexpr auto kTypeCode = kDictObjCode;
		static constexpr auto kClsName = std::string_view{"DictObj"};
		template<typename T> DictObj(const T& dict
			BINON_CONCEPTS_CONSTRUCTOR(
				std::same_as<T BINON_COMMA TDict>,
				std::is_same_v<T BINON_COMMA TDict>,
			);
		template<typename T> DictObj(T&& dict
			BINON_CONCEPTS_CONSTRUCTOR(
				std::same_as<T BINON_COMMA TDict>,
				std::is_same_v<T BINON_COMMA TDict>, noexcept
			);
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
		template<typename T>
			SKDict(const T& value, CodeByte keyCode
			BINON_CONCEPTS_CONSTRUCTOR(
				std::same_as<T BINON_COMMA TDict>,
				std::is_same_v<T BINON_COMMA TDict>,
			);
		template<typename T>
			SKDict(T&& value, CodeByte keyCode
			BINON_CONCEPTS_CONSTRUCTOR(
				std::same_as<T BINON_COMMA TDict>,
				std::is_same_v<T BINON_COMMA TDict>, noexcept
			);
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
		template<typename T>
			SDict(const T& value,
			CodeByte keyCode, CodeByte valCode
			BINON_CONCEPTS_CONSTRUCTOR(
				std::same_as<T BINON_COMMA TDict>,
				std::is_same_v<T BINON_COMMA TDict>,
			);
		template<typename T>
			SDict(T&& value,
			CodeByte keyCode, CodeByte valCode
			BINON_CONCEPTS_CONSTRUCTOR(
				std::same_as<T BINON_COMMA TDict>,
				std::is_same_v<T BINON_COMMA TDict>, noexcept
			);
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

	//==== Template Implementation =============================================

	//---- DictObj -------------------------------------------------------------

	template<typename T> DictObj::DictObj(const T& dict
		BINON_CONCEPTS_CONSTRUCTOR_DEF(
			std::same_as<T BINON_COMMA TDict>,
			std::is_same_v<T BINON_COMMA TDict>,
		)
	{
		this->mValue = dict;
	}
	template<typename T> DictObj::DictObj(T&& dict
		BINON_CONCEPTS_CONSTRUCTOR_DEF(
			std::same_as<T BINON_COMMA TDict>,
			std::is_same_v<T BINON_COMMA TDict>, noexcept
		)
	{
		this->mValue = std::move(dict);
	}

	//---- SKDict --------------------------------------------------------------

	template<typename T>
		SKDict::SKDict(const T& value, CodeByte keyCode
		BINON_CONCEPTS_CONSTRUCTOR_DEF(
			std::same_as<T BINON_COMMA TDict>,
			std::is_same_v<T BINON_COMMA TDict>,
		): mKeyCode{keyCode}
	{
		this->mValue = value;
	}
	template<typename T>
		SKDict::SKDict(T&& value, CodeByte keyCode
		BINON_CONCEPTS_CONSTRUCTOR_DEF(
			std::same_as<T BINON_COMMA TDict>,
			std::is_same_v<T BINON_COMMA TDict>, noexcept
		): mKeyCode{keyCode}
	{
		this->mValue = std::move(value);
	}

	//---- SDict --------------------------------------------------------------

	template<typename T>
		SDict::SDict(const T& value, CodeByte keyCode, CodeByte valCode
		BINON_CONCEPTS_CONSTRUCTOR_DEF(
			std::same_as<T BINON_COMMA TDict>,
			std::is_same_v<T BINON_COMMA TDict>,
		): mKeyCode{keyCode}, mValCode{valCode}
	{
		this->mValue = value;
	}
	template<typename T>
		SDict::SDict(T&& value, CodeByte keyCode, CodeByte valCode
		BINON_CONCEPTS_CONSTRUCTOR_DEF(
			std::same_as<T BINON_COMMA TDict>,
			std::is_same_v<T BINON_COMMA TDict>, noexcept
		): mKeyCode{keyCode}, mValCode{valCode}
	{
		this->mValue = std::move(value);
	}
}

#endif
