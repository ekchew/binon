#ifndef BINON_BINONOBJ_HPP
#define BINON_BINONOBJ_HPP

#include "nullobj.hpp"
#include "boolobj.hpp"
#include "intobj.hpp"
#include "floatobj.hpp"
#include "bufferobj.hpp"
#include "strobj.hpp"
#include "listobj.hpp"
#include "dictobj.hpp"
#include "optutil.hpp"
#include <functional>
#include <optional>
#include <ostream>
#include <type_traits>
#include <variant>

namespace binon {

	//	BinONVariant is a std::variant of all the BinON object types.
	using BinONVariant = std::variant<
		NullObj,
		BoolObj,
		IntObj,
		UIntObj,
		FloatObj,
		Float32Obj,
		BufferObj,
		StrObj,
		ListObj,
		SList,
		DictObj,
		SKDict,
		SDict
		>;

	//	kIsObj<T> tells you if your type T is one of the above BinON object
	//	variants.
	template<typename T>
		constexpr bool kIsObj
			= kIsVariantMember<std::decay_t<T>,BinONVariant>;
 BINON_IF_CONCEPTS(
	template<typename T> concept ObjType = kIsObj<T>;
 )

	// he BinONObj struct is central to the BinON C++ implementation. As a
	// BinONVariant, it can be any of the object types. BinONObjs are managed by
	// many of the APIs in this library, and the struct itself comes with some
	// convenience methods for encoding/decoding BinON and so forth.
	struct BinONObj: BinONVariant {
		using TValue = BinONVariant;
		auto& value() & { return *static_cast<TValue*>(this); }
		const auto& value() const& { return *static_cast<const TValue*>(this); }
		auto value() && { return std::move(*static_cast<TValue*>(this)); }

		//	The Decode() class method can decode an arbitrary BinON object from
		//	an input binary stream.
		//
		//	Args:
		//		stream: a std::istream from which raw bytes can be read as char
		//		requireIO: throw exception if a read operation fails?
		//			This argument is common to all encoding/decoding methods and
		//			always defaults to true. It sets the various std::ios
		//			exception flags. Generally, you want this.
		//
		//			If you are doing several encoding/decoding operations in a
		//			row, however, you might consider declaring a RequireIO
		//			object (see ioutil.hpp). This will set the flags while it
		//			exists and restore them when it destructs. Then you can call
		//			BinONObj::Decode(stream, kSkipRequireIO) since you know the
		//			exception bits have already been set.
		static auto Decode(TIStream& stream, bool requireIO = true) -> BinONObj;

		//	FromTypeCode() returns a new object of the type specified by type
		//	code (see codebyte.hpp for a list of possible codes). This object
		//	type will be default-constructed.
		//
		//	Decode() calls FromTypeCode() internally after reading the type code
		//	byte from the data stream. You may never need to call it yourself.
		static auto FromTypeCode(CodeByte typeCode) -> BinONObj;

		//	A BinON object can be instantiated in several ways. Any std::variant
		//	constructors should work. For example, you could have it contain a
		//	string with:
		//
		//		BinONObj obj{StrObj{"foo"}};
		//
		//	Using FromTypeCode(), this would look like:
		//
		//		auto obj = BinONObj::FromTypeCode{kStrObjCode};
		//		std::get<StrObj>(obj).value() = "foo";
		//
		//	Perhaps the easiest way is to call MakeObj() (see
		//	typeconv.hpp):
		//
		//	auto obj = MakeObj("foo");
		using BinONVariant::variant;

		//	typeCode() returns the BinON type code associated with the current
		//	object type. For example, you could write:
		//
		//		bool isStrObj = obj.typeCode() == kStrObjCode;
		//
		//	Alternatively, you could use std::holds_alternative():
		//
		//		bool isStrObj = std::holds_alternative<StrObj>(obj);
		auto typeCode() const -> CodeByte;

		//	As you would expect, the encode() method encodes a BinON
		//	representation of the current object to an output binary stream.
		//
		//	Note that each specific BinON object class (e.g. StrObj) has its own
		//	specialized encode() method. This encode() simply calls the
		//	appropriate one.
		auto encode(TOStream& stream, bool requireIO = true) const
			-> const BinONObj&;

		//	encodeData() omits encoding the type code and goes straight to the
		//	object data. You can call this if the object type is clear by
		//	context. For example, the ListObj class encodes the length of the
		//	list with UIntObj's encodeData(). It doesn't need to encode the type
		//	since the size of a list will always be an unsigned integer.
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const BinONObj&;

		//	decodeData() reads back what encodeData() wrote. Note how it is an
		//	instance method--not a class method like Decode(). That means you
		//	need to have allocated a BinONObj of the correct type before you can
		//	call decodeData() on it.
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> BinONObj&;

		//	asObj() attempts to extract a specific object type from a general
		//	BinONObj. If your primary choice does not pan out, it may run
		//	through alternates which it will attempt to static_cast into the
		//	primary type.
		//
		//	The following type conversions are officially supported:
		//
		//		UIntObj -> IntObj
		//			If value is > 2 ^ 63, it will be stored in vector form in
		//			the IntObj.
		//		IntObj -> UIntObj
		//			Throws std::out_of_range if IntObj holds negative value
		//		Float32Obj -> FloatObj
		//		SList -> ListObj
		//		SKDict -> DictObj
		//		SDict -> DictObj
		//
		//	If you use move semantics, only your primary object type (the first
		//	and only required template argument) can be moved. To request a
		//	primary object move, try something like this:
		//
		//		auto list = std::move(myBinONObj).asObj<ListObj,SList>;
		//
		//	Note that asObj() is cslled automatically by TypeConv and the
		//	various helper functions that depend on it, so you may never need to
		//	call it directly.
		template<typename Obj, typename... Alts>
			auto asObj() const& BINON_CONCEPTS_FN(
				ObjType<Obj> && (ObjType<Alts> && ...),
				kIsObj<Obj> && (kIsObj<Alts> && ...),
				Obj // return type
			);
		template<typename Obj, typename... Alts>
			auto asObj() && BINON_CONCEPTS_FN(
				ObjType<Obj> && (ObjType<Alts> && ...),
				kIsObj<Obj> && (kIsObj<Alts> && ...),
				Obj // return type
			);

		//	The print() prints a textual description of a BinONObj to an output
		//	text stream. This is somewhat reminiscent of printing an object's
		//	repr() string in Python, and can be helpful in debugging.
		//
		//	For example:
		//
		//		BinONObj obj{StrObj{"foo"}}
		//		obj.print();
		//
		//	should print:
		//
		//		StrObj("foo")
		//
		//	Alternatively, you can use C++ iostream syntax and print it with:
		//
		//	std::cout << obj;
		//
		//	Args:
		//		stream: an optional output text stream
		//			If not supplied, std::cout will be used.
		void print(OptRef<std::ostream> stream = std::nullopt) const;

	private:
		template<typename Obj>
			auto tryAlts() const& BINON_CONCEPTS_FN(
				ObjType<Obj>, kIsObj<Obj>, Obj
			);
		template<typename Obj, typename Alt, typename... Alts>
			auto tryAlts() const& BINON_CONCEPTS_FN(
				ObjType<Obj> && ObjType<Alt> && (ObjType<Alts> && ...),
				kIsObj<Obj> && kIsObj<Alt> && (kIsObj<Alts> && ...),
				Obj
			);
	};
	auto operator<< (std::ostream& stream, const BinONObj& obj)
		-> std::ostream&;

	//	See also BinON object helper functions in objhelpers.hpp.
}

namespace std {

	//	The C++ implementation now supports hashing (and equivalence-comparing)
	//	of any kind of BinONObj. That means you can use BinONObj as the key in
	//	any of the std::unordered... containers. Since DictObj, SKDict, and
	//	SDict are built around st::unordered_map, you can use a BinONObj as
	//	either key or value.
	//
	//	Note that specific BinON object types like IntObj and so on do not
	//	support std::hash directly at this point. The do implement a hash()
	//	method, however. BinONObj's std::hash implementation calls hash()
	//	internally.
	template<> struct hash<binon::BinONObj> {
		auto operator() (const binon::BinONObj& obj) const -> std::size_t;
	};

}

//==== Template Implementation =================================================

namespace binon {

	template<typename Obj, typename... Alts>
		auto BinONObj::asObj() const& BINON_CONCEPTS_FN(
			ObjType<Obj> && (ObjType<Alts> && ...),
			kIsObj<Obj> && (kIsObj<Alts> && ...),
			Obj
		)
	{
		if(auto pObj = std::get_if<Obj>(this); pObj) {
			return *pObj;
		}
		else {
			return tryAlts<Obj,Alts...>();
		}
	}
	template<typename Obj, typename... Alts>
		auto BinONObj::asObj() && BINON_CONCEPTS_FN(
			ObjType<Obj> && (ObjType<Alts> && ...),
			kIsObj<Obj> && (kIsObj<Alts> && ...),
			Obj
		)
	{
		if(auto pObj = std::get_if<Obj>(this); pObj) {
			return std::move(*pObj);
		}
		else {
			return tryAlts<Obj,Alts...>();
		}
	}
	template<typename Obj>
		auto BinONObj::tryAlts() const& BINON_CONCEPTS_FN(
			ObjType<Obj>, kIsObj<Obj>, Obj
		) {
			return std::get<Obj>(*this);
				// should throw std::bad_variant_access
		}
	template<typename Obj, typename Alt, typename... Alts>
		auto BinONObj::tryAlts() const& BINON_CONCEPTS_FN(
			ObjType<Obj> && ObjType<Alt> && (ObjType<Alts> && ...),
			kIsObj<Obj> && kIsObj<Alt> && (kIsObj<Alts> && ...),
			Obj
		) {
			auto pAlt = std::get_if<Alt>(this);
			if(pAlt) {
				return static_cast<Obj>(pAlt->value());
			}
			else {
				return tryAlts<Obj,Alts...>();
			}
		}
}

#endif
