#ifndef BINON_LISTHELPERS_HPP
#define BINON_LISTHELPERS_HPP

#include "objhelpers.hpp"

namespace binon {

	//	kIsListType<T> indicates whether type T (or a decayed form thereof) is a
	//	ListObj or SList. It is used by several functions defined in this
	//	header.
	template<typename T>
		constexpr bool kIsListType
			= std::is_base_of_v<ListBase, std::decay_t<T>>;
 BINON_IF_CONCEPTS(
	template<typename T>
		concept ListType = kIsListType<T>;
 )

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

 #if BINON_CONCEPTS

	//	GetCtnrVal() is essentially GetObjVal() as applied to a particular list
	//	element. (Note that dicthelpers.hpp also implements GetCtnrVal and other
	//	"Ctnr" functions that take keys rather than element indices.) Note that
	//	GetCtnrVal() may throw std::out_of_range if the index is bad (see
	//	std::vector::at() documentation). A std::bad_variant_access exception is
	//	also possible if the type T doesn't work out.
	template<typename T, ListType List>
		 auto GetCtnrVal(const List& list, std::size_t index) -> TGetObjVal<T>;

	//	Likewise, CtnrTValue() is like ObjTValue() applied to a list element. To
	//	move a TValue out of an element, call std::move on the list itself. For
	//	example:
	//
	//		auto hyStr = CtnrTValue<StrObj::TValue>(std::move(myList), 0);
	//
	//	Now, assuming element 0 of myList is indeed a string, hyStr should
	//	contain its text and the element's string should be empty.
	template<TValueType T, ListType List>
		auto CtnrTValue(List& list, std::size_t index) -> T&;
	template<TValueType T, ListType List>
		auto CtnrTValue(const List& list, std::size_t index) -> const T&;
 	template<TValueType T, ListType List>
 		auto CtnrTValue(List&& list, std::size_t index) -> T;

	//	SetCtnrVal() is equivalent to assigning a new object created by calling
	//	MakeObj(v) to an existing list element. (Note that it and AppendVal
	//	return the input list so that you can chain several calls together using
	//	the fluent paradigm.)
	template<ListType List, typename Val>
		auto SetCtnrVal(List& list, std::size_t index, const Val& v) -> List&;

	//	AppendVal() adds elements to the list. You can use move semantics if the
	//	type is a TValue.
	template<ListType List, typename T>
		auto AppendVal(List& list, const T& v) -> List&;
	template<ListType List, TValueType T>
		auto AppendVal(List& list, T&& v) -> List&;
 #endif

	//==== Template Implementation =============================================

	//---- CtnrTValue function templates ---------------------------------------

 #if BINON_CONCEPTS
	template<TValueType T, ListType List>
		auto CtnrTValue(List& list, std::size_t index) -> T&
	{
		return ObjTValue<T>(list.value().at(index));
	}
	template<TValueType T, ListType List>
		auto CtnrTValue(const List& list, std::size_t index) -> const T&
	{
		return ObjTValue<T>(list.value().at(index));
	}
 	template<TValueType T, ListType List>
 		auto CtnrTValue(List&& list, std::size_t index) -> T
 	{
 		return ObjTValue<T>(std::move(list.value().at(index)));
 	}
#else
	template<typename T, typename List>
		auto CtnrTValue(List& list, std::size_t index)
		-> std::enable_if<kIsTValue<T> && kIsListType<List>, T&>
	{
		return ObjTValue<T>(list.value().at(index));
	}
	template<typename T, typename List>
		auto CtnrTValue(const List& list, std::size_t index)
		-> std::enable_if<kIsTValue<T> && kIsListType<List>, const T&>
	{
		return ObjTValue<T>(list.value().at(index));
	}
 	template<typename T, typename List>
 		auto CtnrTValue(List&& list, std::size_t index)
		-> std::enable_if<kIsTValue<T> && kIsListType<List>, T>
 	{
 		return ObjTValue<T>(std::move(list.value().at(index)));
 	}
 #endif

	//---- GetCtnrVal function templates ---------------------------------------

 #if BINON_CONCEPTS
	template<typename T, ListType List>
		auto GetCtnrVal(const List& list, std::size_t index) -> TGetObjVal<T>
	{
		return GetObjVal<T>(list.value().at(index));
	}
 #else
	template<typename T, typename List>
		auto GetCtnrVal(const List& list, std::size_t index)
		-> std::enable_if_t<
			kIsListType<List>,
			TGetObjVal<T>
			>
	{
		return GetObjVal<T>(list.value().at(index));
	}
 #endif

	//---- SetCtnrVal function templates ---------------------------------------

 #if BINON_CONCEPTS
	auto& SetCtnrVal(ListType auto& list, std::size_t index, const auto& v)
 #else
 	template<typename List, typename T>
		auto SetCtnrVal(List& list, std::size_t index, const T& v)
			-> std::enable_if_t<kIsListType<List>, List&>
 #endif
	{
		list.value().at(index) = MakeObj(v);
		return list;
	}

	//---- AppendVal function templates ----------------------------------------

#if BINON_CONCEPTS
	auto& AppendVal(ListType auto& list, const auto& v) {
		list.value().push_back(MakeObj(v));
		return list;
	}
	template<ListType List, TValueType T> auto& AppendVal(List& list, T&& v) {
		list.value().push_back(MakeObj(std::forward<T>(v)));
		return list;
	}
#else
	template<
		typename List, typename T,
		typename std::enable_if_t<kIsListType<List>, int> = 0
		>
		auto& AppendVal(List& list, const T& v)
	{
		list.value().push_back(MakeObj(v));
		return list;
	}
	template<
		typename List, typename T,
		typename std::enable_if_t<
			kIsListType<List> && kIsTValue<T>, int
			> = 0
		>
		auto& AppendVal(List& list, T&& v)
	{
		list.value().push_back(MakeObj(std::forward<T>(v)));
		return list;
	}
#endif

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
