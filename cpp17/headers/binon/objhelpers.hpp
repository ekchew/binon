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
	//		auto intObj = MakeObj(42);
	//
	//	is equivalent to:
	//
	//		IntObj intObj{42};
	//
	//	In other words, it automatically identifies that IntObj is the best
	//	match for 42 using TypeConv.
	//
	//	If the data type of your arguement is a TValue (the value storage type
	//	used by a particular BinON object class), you can use move semantics.
	//
	//		HyStr foo{"foo"};
	//		auto strObj = MakeObj(std::move(foo));
	//
	//	This works because HyStr is StrObj::TValue.
	//
	template<typename T> auto MakeObj(const T& v) -> TValObj<T>;
	template<TValueType T> auto MakeObj(T&& v) noexcept -> TValObj<T>;

	//	GetObjVal() attempts to extract a value of the type you specify from a
	//	general BinONObj. It can accept any type recognized by TypeConv, and can
	//	even perform some basic object type conversions (see BinONObj::asObj())
	//	in an effort to get your value. It will, however, throw an exception if
	//	it cannot find a way.
	//
	//	Example:
	//		BinONObj obj = MakeObj(42);
	//		std::cout << GetObjVal<unsigned int>(obj) << '\n';
	//
	//	Output:
	//		42
	//
	//	Consider what GetObjVal() effectively has to do make this happen. Behind
	//	the scenes, it's doing something like this automatically:
	//
	//		static_cast<unsigned int>(
	//			static_cast<UIntObj>(std::get<IntObj>(obj)).value().scalar()
	//			)
	//
	template<typename T>
		auto GetObjVal(const BinONObj& obj) -> TGetObjVal<T>;

	//	ObjTValue() is a more focused alternative to GetObjVal(). In this case,
	//	the type T must be the TValue type of specific object stored in the
	//	BinONObj.
	//
	//	But by supplying this, you get more direct access to the stored value.
	//	You can get a reference to the TValue that lets you modify the object
	//	value in-place. You can also move the value out of the object.
	template<TValueType T>
		auto ObjTValue(BinONObj& obj) -> T&;
	template<TValueType T>
		auto ObjTValue(const BinONObj& obj) -> const T&;
	template<TValueType T>
		auto ObjTValue(BinONObj&& obj) -> T;

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
		ObjWrapper(TValueType auto&& val);
	 #else
		template<typename T, typename std::enable_if_t<kIsTValue<T>,int> = 0>
			ObjWrapper(T&& val);
	 #endif
	};

	//==== Template Implementation =============================================

	//---- MakeObj -------------------------------------------------------------
	//
	//	There are 3 overloads for MakeObj. One deals with copy semantics, one
	//	with move semantics, and the 3rd handles the special case of C string
	//	literals (const char*) which seem to have some issues with reference
	//	resolution.

	template<typename T> auto MakeObj(const T& v) -> TValObj<T> {
		if constexpr(kIsCStr<T>) {
			return HyStr(v);
		}
		else {
			return TValObj<T>(v);
		}
	}

 #if BINON_CONCEPTS
	template<TValueType T>
		auto MakeObj(T&& v) noexcept -> TValObj<T>
 #else
	template<typename T>
		auto MakeObj(T&& v) noexcept
			-> std::enable_if_t<kIsTValue<T>, TValObj<T>>
 #endif
	{
		return TValObj<T>(std::forward<T>(v));
	}

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
