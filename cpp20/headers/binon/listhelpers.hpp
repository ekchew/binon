#ifndef BINON_LISTHELPERS_HPP
#define BINON_LISTHELPERS_HPP

#include "objhelpers.hpp"
#include <initializer_list>

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
	//		auto list = MakeListObj({42u, "foo", 3.14f});
	//
	//	should return a ListObj containing a UIntObj, a StrObj, and a
	//	Float32Obj.
	//
	auto MakeListObj(std::initializer_list<ObjWrapper> vals) -> ListObj;
	auto MakeSList(CodeByte elemCode, std::initializer_list<ObjWrapper> vals)
		-> SList;

	//	GetCtnrVal() is essentially GetObjVal() as applied to a particular list
	//	element. (Note that dicthelpers.hpp also implements GetCtnrVal and other
	//	"Ctnr" functions that take keys rather than element indices.) Note that
	//	GetCtnrVal() may throw std::out_of_range if the index is bad (see
	//	std::vector::at() documentation). A std::bad_variant_access exception is
	//	also possible if the type T doesn't work out.
	//
	//template<TCType T, ListType List>
	//	 auto GetCtnrVal(const List& list, std::size_t index) -> TGetObjVal<T>;

	//	Likewise, CtnrTValue() is like ObjTValue() applied to a list element. To
	//	move a TValue out of an element, call std::move on the list itself. For
	//	example:
	//
	//		auto hyStr = CtnrTValue<StrObj::TValue>(std::move(myList), 0);
	//
	//	Now, assuming element 0 of myList is indeed a string, hyStr should
	//	contain its text and the element's string should be empty.
	//
	//TValueType auto& CtnrTValue(ListType auto& list, std::size_t index);
	//const TValueType auto& CtnrTValue(
	//		const ListType auto& list, std::size_t index
	//	);
 	//TValueType auto CtnrTValue(ListType auto&& list, std::size_t index);

	//	SetCtnrVal() is equivalent to assigning a new object created by calling
	//	MakeObj(v) to an existing list element. (Note that it and AppendVal
	//	return the input list so that you can chain several calls together using
	//	the fluent paradigm.)
	//
	//template<ListType List, TCType Val>
	//	auto SetCtnrVal(List& list, std::size_t index, const Val& v) -> List&;

	//	AppendVal() adds elements to the list. You can use move semantics if the
	//	type is a TValue.
	//
	//template<ListType List, TCType T>
	//	auto AppendVal(List& list, const T& v) -> List&;
	//template<ListType List, TCType T>
	//	auto AppendVal(List& list, T&& v) -> List&;

	//==== Template Implementation =============================================

	//---- CtnrTValue function templates ---------------------------------------

	template<typename T, typename List>
		auto CtnrTValue(List& list, std::size_t index)
		BINON_CONCEPTS_FN(
			TValueType<T> && ListType<List>,
			kIsTValue<T> && kIsListType<List>,
			T&
		)
	{
		return ObjTValue<T>(list.value().at(index));
	}
	template<typename T, typename List>
		auto CtnrTValue(const List& list, std::size_t index)
		BINON_CONCEPTS_FN(
			TValueType<T> && ListType<List>,
			kIsTValue<T> && kIsListType<List>,
			const T&
		)
	{
		return ObjTValue<T>(list.value().at(index));
	}
	template<typename T, typename List>
		auto CtnrTValue(List&& list, std::size_t index)
		BINON_CONCEPTS_FN(
			TValueType<T> && ListType<List>,
			kIsTValue<T> && kIsListType<List>,
			T
		)
 	{
 		return ObjTValue<T>(std::move(list.value().at(index)));
 	}

	//---- GetCtnrVal function templates ---------------------------------------

 	template<typename T, typename List>
		auto GetCtnrVal(const List& list, std::size_t index)
		BINON_CONCEPTS_FN(
			TCType<T> && ListType<List>,
			kIsTCType<T> && kIsListType<List>,
			TGetObjVal<T>
		)
	{
		return GetObjVal<T>(list.value().at(index));
	}

	//---- SetCtnrVal function templates ---------------------------------------

 	template<typename List, typename T>
		auto SetCtnrVal(List& list, std::size_t index, const T& v)
		BINON_CONCEPTS_FN(
			ListType<List> && TCType<T>,
			kIsListType<List> && kIsTCType<T>,
			List&
		)
	{
		auto& elem = list.value().at(index);
		if constexpr(std::is_base_of_v<SList,List>) {
			elem = MakeTypeCodeObj(list.mElemCode, v);
		}
		else {
			elem = MakeObj(v);
		}
		return list;
	}

	//---- AppendVal function templates ----------------------------------------

	template<typename List, typename T>
		auto AppendVal(List& list, const T& v)
		BINON_CONCEPTS_FN(
			ListType<List> && TCType<T>,
			kIsListType<List> && kIsTCType<T>,
			List&
		)
	{
		if constexpr(std::is_base_of_v<SList,List>) {
			list.value().push_back(MakeTypeCodeObj(list.mElemCode, v));
		}
		else {
			list.value().push_back(MakeObj(v));
		}
		return list;
	}
	template<typename List, typename T>
		auto AppendVal(List& list, T&& v)
		BINON_CONCEPTS_FN(
			ListType<List> && TValueType<T>,
			kIsListType<List> && kIsTValue<T>,
			List&
		)
	{
		if constexpr(std::is_base_of_v<SList,List>) {
			list.value().push_back(MakeTypeCodeObj(list.mElemCode, std::move(v)));
		}
		else {
			list.value().push_back(MakeObj(std::move(v)));
		}

		return list;
	}

}

#endif
