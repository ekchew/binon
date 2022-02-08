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

	//	CntrTValue() is essentially ObjTValue() (see objhelpers.hpp) applied to
	//	a specific container element.
	template<TValueType T, ListType List>
		auto CtnrTValue(List& list, std::size_t index) -> T&;
	template<TValueType T, ListType List>
		auto CtnrTValue(const List& list, std::size_t index) -> const T&;
 	template<TValueType T, ListType List>
 		auto CtnrTValue(List&& list, std::size_t index) -> T;

	//	Likewise, GetCntrVal() is the container element counterpart to
	//	GetObjVal().
	template<typename T, ListType List>
		 auto GetCtnrVal(const List& list, std::size_t index) -> TGetObjVal<T>;

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
	auto& SetCtnrVal(ListType auto& list, std::size_t index, const char* s) {
		list.value().at(index) = MakeObj(s);
		return list;
	}
	auto& SetCtnrVal(
		ListType auto& list, std::size_t index, const NonCStr auto& v
	) {
		list.value().at(index) = MakeObj(v);
		return list;
	}
 #else
	template<
		typename List,
		typename std::enable_if_t<kIsListType<List>, int> = 0
		>
		auto& SetCtnrVal(List& list, std::size_t index, const char* s)
	{
		list.value().at(index) = MakeObj(s);
		return list;
	}
	template<
		typename List, typename T,
		typename std::enable_if_t<kIsListType<List> && !kIsCStr<T>, int> = 0
		>
		auto& SetCtnrVal(List& list, std::size_t index, const T& v)
	{
		list.value().at(index) = MakeObj(v);
		return list;
	}
 #endif

	//---- AppendVal function templates ----------------------------------------

#if BINON_CONCEPTS
	auto& AppendVal(ListType auto& list, const char* s) {
		list.value().push_back(MakeObj(s));
		return list;
	}
	auto& AppendVal(ListType auto& list, const NonCStr auto& v) {
		list.value().push_back(MakeObj(v));
		return list;
	}
	template<ListType List, NonCStr T> auto& AppendVal(List& list, T&& v) {
		list.value().push_back(MakeObj(std::forward<T>(v)));
		return list;
	}
#else
	template<
		typename List,
		typename std::enable_if_t<kIsListType<List>, int> = 0
		>
		auto& AppendVal(List& list, const char* s)
	{
		list.value().push_back(MakeObj(s));
		return list;
	}
	template<
		typename List, typename T,
		typename std::enable_if_t<kIsListType<List> && !kIsCStr<T>, int> = 0
		>
		auto& AppendVal(List& list, const T& v)
	{
		list.value().push_back(MakeObj(v));
		return list;
	}
	template<
		typename List, typename T,
		typename std::enable_if_t<
			kIsListType<List> && kIsTValue<T> && !kIsCStr<T>, int
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
