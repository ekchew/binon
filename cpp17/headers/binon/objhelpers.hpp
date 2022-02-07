#ifndef BINON_OBJHELPERS_HPP
#define BINON_OBJHELPERS_HPP

#include "typeconv.hpp"

namespace binon {

	//	TGetObjVal<T> is the core type returned by the GetObjVal<T>()
	//	function. In many cases, TGetObjVal<T> is simply T. See TypeConv's
	//	TVal type for more info.
	template<typename T>
		using TGetObjVal = typename TypeConv<std::decay_t<T>>::TVal;

 #if BINON_CONCEPTS

	//	MakeObj() is a convenience function to build the appropriate type of
	//	object for the argument you supply. For example:
	//
	//		auto obj = MakeObj("foo");
	//
	//	is equivalent to:
	//
	//		StrObj obj{"foo"};
	//
	//	In other words, it automatically identifies that StrObj is the best
	//	match for "foo" using TypeConv.
	//
	//	If the data type of your arguement is a TValue (the value storage type
	//	used by a particular BinON object class), you can use move semantics.
	//
	//		HyStr foo{"foo"}; auto obj = MakeObj(std::move(foo));
	//
	//	This works because HyStr is StrObj::TValue.
	auto MakeObj(const char* s) -> StrObj;
	template<NonCStr T> auto MakeObj(const T& v) -> TValObj<T>;
	template<TValueType T> auto MakeObj(T&& v) noexcept -> TValObj<T>;

	//	ObjTValue() gives you direct access to an object's TValue, but you must
	//	choose the correct one to avoid a std::bad_variant_access exception.
	//	Direct access means you can receive a reference to the TValue you can
	//	modify in-place (provided the object is not const). You can also move
	//	the TValue out of the BinONObj.
	template<TValueType T>
		auto ObjTValue(BinONObj& obj) -> T&;
	template<TValueType T>
		auto ObjTValue(const BinONObj& obj) -> const T&;
	template<TValueType T>
		auto ObjTValue(BinONObj&& obj) -> T;

	//	This is a more permissive alternative to ObjTValue. It can accept any
	//	type recognized by TypeConv and even handle simple conversions like
	//	UIntObj to IntObj (see BinONObj::asObj()). On the other hand, it always
	//	returns a copy, so there is a bit more overhead.
	template<NonTValue T>
		auto GetObjVal(const BinONObj& obj) -> TGetObjVal<T>;

 #endif

	//	ObjWrapper offers an alternative to MakeObj as a means of converting an
	//	arbitrary value into a BinONObj. You can have one of these be a function
	//	argument. The caller can then supply any value that is convertible and
	//	your function should receive a BinONObj as its argument.
	struct ObjWrapper: BinONObj {
		ObjWrapper(const BinONObj& obj);
		ObjWrapper(BinONObj&& obj) noexcept;
		ObjWrapper(const char* cStr);
		template<typename T>
			ObjWrapper(const T& val);
	 #if BINON_CONCEPTS
		template<TValueType T>
	 #else
		template<typename T, typename std::enable_if_t<kIsTValue<T>,int> = 0>
	 #endif
			ObjWrapper(T&& val);
	};

	//==== Template Implementation =============================================

	//---- MakeObj -------------------------------------------------------------
	//
	//	There are 3 overloads for MakeObj. One deals with copy semantics, one
	//	with move semantics, and the 3rd handles the special case of C string
	//	literals (const char*) which seem to have some issues with reference
	//	resolution.

	auto MakeObj(const char* s) -> StrObj;

 #if BINON_CONCEPTS
	template<NonCStr T> auto MakeObj(const T& v) -> TValObj<T> {
		return TValObj<T>(v);
	}
	template<TValueType T> auto MakeObj(T&& v) noexcept -> TValObj<T> {
		return TValObj<T>(std::forward<T>(v));
	}
 #else
	template<typename T>
		auto MakeObj(const T& v)
			-> std::enable_if_t<!kIsCStr<T>, TValObj<T>>
	{
		return TValObj<T>(v);
	}
	template<typename T>
		auto MakeObj(T&& v) noexcept
			-> std::enable_if_t<kIsTValue<T>, TValObj<T>>
	{
		return TValObj<T>(std::forward<T>(v));
	}
 #endif

	//---- ObjTValue -----------------------------------------------------------

 #if BINON_CONCEPTS
	template<TValueType T>
		auto ObjTValue(BinONObj& obj) -> T&
	{
		return std::get<typename TypeConv<T>::TObj>(obj).value();
	}
	template<TValueType T>
		auto ObjTValue(const BinONObj& obj) -> const T&
	{
		return std::get<typename TypeConv<T>::TObj>(obj).value();
	}
	template<TValueType T>
		auto ObjTValue(BinONObj&& obj) -> T
	{
		return std::move(std::get<typename TypeConv<T>::TObj>(obj)).value();
	}
 #else
	template<typename T>
		auto ObjTValue(BinONObj& obj) -> std::enable_if_t<kIsTValue<T>, T&>
	{
		return std::get<typename TypeConv<T>::TObj>(obj).value();
	}
	template<typename T>
		auto ObjTValue(const BinONObj& obj)
		-> std::enable_if_t<kIsTValue<T>, const T&>
	{
		return std::get<typename TypeConv<T>::TObj>(obj).value();
	}
	template<typename T>
		auto ObjTValue(BinONObj&& obj) -> std::enable_if_t<kIsTValue<T>, T>
	{
		return std::move(std::get<typename TypeConv<T>::TObj>(obj)).value();
	}
 #endif

	//---- GetObjVal ---------------------------------------------------------
	//
	//	Of the 4 overloads of GetObjVal, the first 3 deal with the case where
	//	T is the native TValue value type of the object class in question. The
	//	first 2 give you an L-value reference that is either constant or mutable
	//	depending on whether obj is const. The 3rd returns by value but using
	//	move semantics for when obj is an R-value reference.
	//
	//	The 4th and last overload handles any other non-native types supported
	//	by TypeConv. Here, it returns by value after static-casting to the type
	//	in question (TypeConv handles the static cast).
	//
	//	Note that the type T is decayed to remove const/reference qualifiers and
	//	such before passing the type to TypeConv.

	template<typename T>
		auto GetObjVal(const BinONObj& obj) -> TGetObjVal<T>
	{
		return TypeConv<T>::GetVal(obj);
	}

	//---- ObjWrapper ----------------------------------------------------------

	template<typename T>
		ObjWrapper::ObjWrapper(const T& val):
			BinONObj{TValObj<T>(val)}
		{
		}
	 #if BINON_CONCEPTS
		template<TValueType T>
	 #else
		template<typename T, typename std::enable_if_t<kIsTValue<T>,int>>
	 #endif
		ObjWrapper::ObjWrapper(T&& val):
			BinONObj{TValObj<T>(std::move(val))}
		{
		}
}

#endif
