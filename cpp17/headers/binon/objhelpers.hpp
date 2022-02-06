#ifndef BINON_OBJHELPERS_HPP
#define BINON_OBJHELPERS_HPP

#include "typeconv.hpp"

namespace binon {
	//	This header defines 2 functions that build on the typeconv.hpp to allow
	//	you to work with more familiar data types like int or float as opposed
	//	than IntObj or Float32Obj. They are MakeBinObj() and BinONObjVal().
	//
	//	These will be described here with idealized function prototypes that
	//	should be sufficient to give you an idea of how to work with them. (In
	//	the actual implementation, the functions are overloaded in various ways
	//	to support move semantics and so forth. You can peruse the template
	//	implementation below if you're feeling brave!)
	//
	//	template<typename T> auto MakeObj(T value) -> TValObj<T>;
	//
	//		This function produces a BinON object out of any type T recognized
	//		by the TypeConv class. For example:
	//
	//			auto obj = MakeObj("foo");
	//
	//		is functionally equivalent to:
	//
	//			StrObj obj = StrObj{HyStr{"foo"}};
	//
	//	template<typename T>
	//		auto BinONObjVal(BinONObj varObj) -> TBinONObjVal<T>;
	//
	//		This is essentially the opposite of MakeObj(), returning the
	//		object's value in the form you prefer.
	//
	//			auto val = BinONObjVal<std::string_view>(obj);
	//
	//		is functionally equivalent to:
	//
	//			auto val = std::get<StrObj>(obj).value().asView();
	//
	//		Note that if you choose your type T to be the native value type of
	//		the BinON object in question, BinONObjVal() will return a T&
	//		reference so that you can modify the stored value in-place if you
	//		like (provided the object is not declared const, of course).
	//
	//		For example, the native type of a StrObj is StrObj::TValue (which is
	//		HyStr). So you could write:
	//
	//			BinONObjVal<StrObj::TValue>(obj) = "bar";
	//
	//		Note that BinONObjVal() may throw TypeErr if you pick the wrong type
	//		T for the object in question. You might want to check with the
	//		typeCode() method or by calling std::holds_alternative() with
	//		specific object types if you are uncertain.
	//
	//	See also listhelpers.hpp and dicthelpers.hpp.

	//	TBinONObjVal<T> is the core type returned by the BinONObjVal<T>()
	//	function. In many cases, TBinONObjVal<T> is simply T. See TypeConv's
	//	TVal type for more info.
	template<typename T>
		using TBinONObjVal = typename TypeConv<std::decay_t<T>>::TVal;

	//	ObjWrapper offers an alternative to MakeObj as a means of
	//	converting an arbitrary value into a BinONObj. You can have one of these
	//	be a function argument. The caller can then supply any value that is
	//	convertible and your function should receive a BinONObj as its argument.
	struct ObjWrapper: BinONObj {
		ObjWrapper(const BinONObj& obj);
		ObjWrapper(BinONObj&& obj) noexcept;
		ObjWrapper(const char* cStr);
		template<typename T>
			ObjWrapper(const T& val);
	 #if BINON_CONCEPTS
		template<BinONVal T>
	 #else
		template<typename T, typename std::enable_if_t<kIsBinONVal<T>,int> = 0>
	 #endif
			ObjWrapper(T&& val);
	};

	//==== Template Implementation =============================================

	template<typename T>
		ObjWrapper::ObjWrapper(const T& val):
			BinONObj{TValObj<T>(val)}
		{
		}
	 #if BINON_CONCEPTS
		template<BinONVal T>
	 #else
		template<typename T, typename std::enable_if_t<kIsBinONVal<T>,int>>
	 #endif
		ObjWrapper::ObjWrapper(T&& val):
			BinONObj{TValObj<T>(std::move(val))}
		{
		}

	//---- MakeObj -------------------------------------------------------------
	//
	//	There are 3 overloads for MakeObj. One deals with copy semantics, one
	//	with move semantics, and the 3rd handles the special case of C string
	//	literals (const char*) which seem to have some issues with reference
	//	resolution.

 #if BINON_CONCEPTS
	template<NonCStr T> auto MakeObj(const T& v) -> TValObj<T> {
		return TValObj<T>(v);
	}
	template<BinONVal T> auto MakeObj(T&& v) noexcept -> TValObj<T> {
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
			-> std::enable_if_t<kIsBinONVal<T>, TValObj<T>>
	{
		return TValObj<T>(std::forward<T>(v));
	}
 #endif
	auto MakeObj(const char* s) -> StrObj;

	//---- BinONObjVal ---------------------------------------------------------
	//
	//	Of the 4 overloads of BinONObjVal, the first 3 deal with the case where
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

 #if BINON_CONCEPTS
	template<BinONVal T>
		auto BinONObjVal(BinONObj& obj) -> TBinONObjVal<T>&
	{
		return TypeConv<std::decay_t<T>>::GetVal(obj);
	}
	template<BinONVal T>
		auto BinONObjVal(const BinONObj& obj) -> const TBinONObjVal<T>&
	{
		return TypeConv<std::decay_t<T>>::GetVal(obj);
	}
	template<BinONVal T>
		auto BinONObjVal(BinONObj&& obj) -> TBinONObjVal<T>
	{
		return
			TypeConv<std::decay_t<T>>::GetVal(std::forward<BinONObj>(obj));
	}
	template<NonBinONVal T>
		auto BinONObjVal(const BinONObj& obj) -> TBinONObjVal<T>
	{
		return TypeConv<std::decay_t<T>>::GetVal(obj);
	}
 #else
	template<typename T>
		auto BinONObjVal(BinONObj& obj)
			-> std::enable_if_t<kIsBinONVal<T>, TBinONObjVal<T>&>
	{
		return TypeConv<std::decay_t<T>>::GetVal(obj);
	}
	template<typename T>
		auto BinONObjVal(const BinONObj& obj)
			-> std::enable_if_t<kIsBinONVal<T>, const TBinONObjVal<T>&>
	{
		return TypeConv<std::decay_t<T>>::GetVal(obj);
	}
	template<typename T>
		auto BinONObjVal(BinONObj&& obj)
			-> std::enable_if_t<kIsBinONVal<T>, TBinONObjVal<T>>
	{
		return
			TypeConv<std::decay_t<T>>::GetVal(std::forward<BinONObj>(obj));
	}
	template<typename T>
		auto BinONObjVal(const BinONObj& obj)
			-> std::enable_if_t<!kIsBinONVal<T>, TBinONObjVal<T>>
	{
		return TypeConv<std::decay_t<T>>::GetVal(obj);
	}
 #endif
}

#endif
