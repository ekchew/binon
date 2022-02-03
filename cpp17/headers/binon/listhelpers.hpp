#ifndef BINON_LISTHELPERS_HPP
#define BINON_LISTHELPERS_HPP

#include "objhelpers.hpp"

namespace binon {

	//	kIsCtnrType<T> indicates whether (the decayed form of) type T is a
	//	ListObj, SList, DictObj, SKDict, or SDict. It is used by the Size()
	//	function defined in this header.
	template<typename T>
		constexpr bool kIsCtnrType
			= std::is_base_of_v<CtnrType, std::decay_t<T>>;

	//	kIsListType<T> indicates whether (the decayed form of) type T is a
	//	ListObj or SList. It is used by several functions defined in this
	//	header.
	template<typename T>
		constexpr bool kIsListType
			= std::is_base_of_v<ListType, std::decay_t<T>>;

	//	Size() returns the number of elements in a list or dictionary object.
	//
	//template<typename Ctnr> auto Size(const Ctnr& ctnr) -> std::size_t;

	//	These functions create a list object out of any TypeConv-supported
	//	values you pass in. For example:
	//
	//		auto list = MakeListObj(42u, "foo", 3.14f);
	//
	//	should return a ListObj containing a UIntObj, a StrObj, and a
	//	Float32Obj.
	template<typename... Ts>
		auto MakeListObj(Ts&&... values) -> ListObj;
	template<typename... Ts>
		auto MakeSList(CodeByte elemCode, Ts&&... values) -> SList;

	//	GetVal() returns the value of a list element using the data type
	//	you supply as T. For example, you could write:
	//
	//		auto x = GetVal<float>(list, 2);
	//
	//	to retrieve the 3.14f from the earlier MakeListObj() example.
	//
	//	Internally, GetVal() calls BinONObjVal() on the element in question (see
	//	objhelpers.hpp). As such, it is overloaded for native BinON value types.
	//	In other words, you can get a reference to the value you can modify like
	//	this:
	//
	//		GetVal<StrObj::TValue>(list, 1) = "bar";
	//
	//	You can also move an element out of the list like this:
	//
	//		auto hyStr = GetVal<StrObj::TValue>(std::move(list), 1);
	//
	//	Now your hyStr variable should contain the string and element 1 of list
	//	should be an empty string.
	//
	//	template<typename T, typename List>
	//		auto GetVal(const List& list, std::size_t index) -> TBinONObjVal<T>;

	//	SetVal() lets you assign a value of any type recognized by TypeConv to a
	//	particular list element. So you could write:
	//
	//		SetVal(list, 1, "baz");
	//
	//	Internally, SetVal() calls MakeBinONObj() and therefore does support
	//	move semantics for native object value types only.
	//
	//		SetVal(list, 1, std::move(hyStr));
	//
	//	template<typename List, typename T>
	//		auto SetVal(List& list, std::size_t index, const T& v) -> List&;

	//	AppendVal() works much like SetVal() except that it pushes new values
	//	onto the end of the list.
	//
	//	template<typename List, typename T>
	//		auto AppendVal(List& list, const T& v) -> List;

	//==== Template Implementation =============================================

	//---- Size function template ----------------------------------------------

	template<typename Ctnr>
		auto Size(const Ctnr& ctnr)
			-> std::enable_if_t<kIsCtnrType<Ctnr>, std::size_t>
	{
		return ctnr.value().size();
	}

	//---- GetVal function templates -------------------------------------------

	template<typename T, typename List>
		auto GetVal(List& list, std::size_t index)
		-> std::enable_if_t<
			kIsListType<List> && kIsBinONVal<T>,
			TBinONObjVal<T>&
			>
	{
		return BinONObjVal<T>(list.value().at(index));
	}
	template<typename T, typename List>
		auto GetVal(const List& list, std::size_t index)
		-> std::enable_if_t<
			kIsListType<List> && kIsBinONVal<T>,
			const TBinONObjVal<T>&
			>
	{
		return BinONObjVal<T>(list.value().at(index));
	}
	template<typename T, typename List>
		auto GetVal(List&& list, std::size_t index)
		-> std::enable_if_t<
			kIsListType<List> && kIsBinONVal<T>,
			TBinONObjVal<T>
			>
	{
		return BinONObjVal<T>(std::forward<List>(list).value().at(index));
	}
	template<typename T, typename List>
		auto GetVal(const List& list, std::size_t index)
		-> std::enable_if_t<
			kIsListType<List> && !kIsBinONVal<T>,
			TBinONObjVal<T>
			>
	{
		return BinONObjVal<T>(list.value().at(index));
	}

	//---- SetVal function templates -------------------------------------------

	template<
		typename List,
		typename std::enable_if_t<kIsListType<List>, int> = 0
		>
		auto& SetVal(List& list, std::size_t index, const char* s)
	{
		list.value().at(index) = MakeBinONObj(s);
		return list;
	}
	template<
		typename List, typename T,
		typename std::enable_if_t<kIsListType<List> && !kIsCStr<T>, int> = 0
		>
		auto& SetVal(List& list, std::size_t index, const T& v)
	{
		list.value().at(index) = MakeBinONObj(v);
		return list;
	}
	template<
		typename List, typename T,
		typename std::enable_if_t<
			kIsListType<List> && kIsBinONVal<T> && !kIsCStr<T>, int
			> = 0
		>
		auto& SetVal(List& list, std::size_t index, T&& v)
	{
		list.value().at(index) = MakeBinONObj(std::forward<T>(v));
		return list;
	}

	//---- AppendVal function templates ----------------------------------------

	template<
		typename List,
		typename std::enable_if_t<kIsListType<List>, int> = 0
		>
		auto& AppendVal(List& list, const char* s)
	{
		list.value().push_back(MakeBinONObj(s));
		return list;
	}
	template<
		typename List, typename T,
		typename std::enable_if_t<kIsListType<List> && !kIsCStr<T>, int> = 0
		>
		auto& AppendVal(List& list, const T& v)
	{
		list.value().push_back(MakeBinONObj(v));
		return list;
	}
	template<
		typename List, typename T,
		typename std::enable_if_t<
			kIsListType<List> && kIsBinONVal<T> && !kIsCStr<T>, int
			> = 0
		>
		auto& AppendVal(List& list, T&& v)
	{
		list.value().push_back(MakeBinONObj(std::forward<T>(v)));
		return list;
	}

	//---- Make... function templates ------------------------------------------

	template<typename... Ts>
		auto MakeListObj(Ts&&... values) -> ListObj
	{
		ListObj list;
		list.value().reserve(sizeof...(Ts));
		(AppendVal(list, std::forward<Ts>(values)), ...);
		return list;
	}
	template<typename... Ts>
		auto MakeSList(CodeByte elemCode, Ts&&... values) -> SList
	{
		SList list(elemCode);
		list.value().reserve(sizeof...(Ts));
		(AppendVal(list, std::forward<Ts>(values)), ...);
		return list;
	}
}

#endif
