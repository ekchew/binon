#ifndef BINON_DICTHELPERS__HPP
#define BINON_DICTHELPERS__HPP

#include "objhelpers.hpp"

//	dicthelpers.hpp is structured a lot like listhelpers.hpp, so the
//	documentation here will mainly highlight differences.

namespace binon {

	template<typename T>
		constexpr bool kIsDictType = std::is_base_of_v<DictBase, T>;
 BINON_IF_CONCEPTS(
	template<typename T>
		concept DictType = kIsDictType<T>;
 )

 #if BINON_CONCEPTS

	//	FindObj() attempts to locate the key you provide in the dictionary. If
	//	it finds it, it returns a reference to the associated BinONObj in the
	//	optional return argument. Otherwise, it return std::nullopt.
	//
	//	After successfully finding the object, you can then call GetObjVal() or
	//	ObjTValue() to further take it apart (see objhelpers.hpp).
	//
	//	You can also call FindObj() simply to check if a key exists, since an
	//	OptRef evaluates true or false in say an if statement.
	auto FindObj(const DictType auto& dict, const auto& key)
		-> OptRef<const BinONObj>;
	auto FindObj(DictType auto& dict, const auto& key)
		-> OptRef<BinONObj>;

	//	Note that aside from taking a key rather than an index, the dictionary
	//	version of GetCtnrVal() differs from the list counterpart in one other
	//	way. For a non-const dictionary argument, there is an option to
	//	auto-allocate. If you pass kAutoAlloc (true) as the 3rd argument, it
	//	should create a new element in the dictionary if one does not exist for
	//	your key. This element will have a default-constructed value. Without
	//	auto-allocation, GetCtnrVal() will throw std::out_of_range instead on a
	//	missing key.
	template<typename Val, DictType Dict, typename Key>
		auto GetCtnrVal(Dict& dict, const Key& key, bool autoAlloc = false)
		-> TGetObjVal<Val>;
	template<typename Val, DictType Dict, typename Key>
		auto GetCtnrVal(const Dict& dict, const Key& key)
		-> TGetObjVal<Val>;

	template<TValueType Val, DictType Dict, typename Key>
		auto CtnrTValue(
			Dict& dict, const Key& key, bool autoAlloc = false
		) -> Val&;
	template<TValueType Val, DictType Dict, typename Key>
		auto CtnrTValue(const Dict& dict, const Key& key) -> const Val&;
	template<TValueType Val, DictType Dict, typename Key>
		auto CtnrTValue(Dict&& dict, const Key& key) -> Val;

	template<DictType Dict, typename Key, typename Val>
		auto SetCtnrVal(Dict& dict, const Key& key, const Val& val) -> Dict&;

	//	DelKey() deletes the element associated with the key you provide if it
	//	exists in the dictionary. It returns true if the key existed and was
	//	removed or false it there was no such key.
	auto DelKey(DictType auto& dict, const auto& key) -> bool;

 #endif

	constexpr bool kAutoAlloc = true;

	//==== Template Implementation =============================================

	//---- FindObj function templates ------------------------------------------

 #if BINON_CONCEPTS
	auto FindObj(const DictType auto& dict, const auto& key)
		-> OptRef<const BinONObj>
 #else
	template<typename Dict, typename Key>
		auto FindObj(const Dict& dict, const Key& key)
		-> std::enable_if_t<kIsDictType<Dict>, OptRef<const BinONObj>>
 #endif
	{
		auto find = [&dict](const BinONObj& keyObj) -> OptRef<const BinONObj> {
			auto& map = dict.value();
			auto pPair = map.find(keyObj);
			if(pPair != map.end()) {
				return pPair->second;
			}
			return std::nullopt;
		};
		if constexpr(std::is_same_v<Key,BinONObj>) {
			return find(key);
		}
		else {
			return find(MakeObj(key));
		}
	}
 #if BINON_CONCEPTS
	auto FindObj(DictType auto& dict, const auto& key)
		-> OptRef<BinONObj>
 #else
	template<typename Dict, typename Key>
		auto FindObj(Dict& dict, const Key& key)
		-> std::enable_if_t<kIsDictType<Dict>, OptRef<BinONObj>>
 #endif
	{
		auto find = [&dict](const BinONObj& keyObj) -> OptRef<BinONObj> {
			auto& map = dict.value();
			auto pPair = map.find(keyObj);
			if(pPair != map.end()) {
				return pPair->second;
			}
			return std::nullopt;
		};
		if constexpr(std::is_same_v<Key,BinONObj>) {
			return find(key);
		}
		else {
			return find(MakeObj(key));
		}
	}

 	//---- GetCtnrVal function templates ---------------------------------------

 #if BINON_CONCEPTS
	template<typename Val, DictType Dict, typename Key>
		auto GetCtnrVal(Dict& dict, const Key& key, bool autoAlloc)
		-> TGetObjVal<Val>
 #else
	template<typename Val, typename Dict, typename Key>
		auto GetCtnrVal(Dict& dict, const Key& key, bool autoAlloc)
		-> std::enable_if_t<kIsDictType<Dict>, TGetObjVal<Val>>
 #endif
	{
		if(!autoAlloc) {
			return GetCtnrVal(static_cast<const Dict&>(dict), key);
		}
		auto& map = dict.value();
		BinONObj keyObj = MakeObj(key);
		auto pPair = map.find(keyObj);
		if(pPair == map.end()) {
			pPair = map.insert(*pPair, {std::move(keyObj), TValObj<Val>()});
		}
		return GetObjVal<Val>(pPair->second);
	}

 #if BINON_CONCEPTS
	template<typename Val, DictType Dict, typename Key>
		auto GetCtnrVal(const Dict& dict, const Key& key)
		-> TGetObjVal<Val>
 #else
	template<typename Val, typename Dict, typename Key>
		auto GetCtnrVal(const Dict& dict, const Key& key)
		-> std::enable_if_t<kIsDictType<Dict>, TGetObjVal<Val>>
 #endif
	{
		return GetObjVal<Val>(dict.value().at(MakeObj(key)));
	}

	//---- CtnrTValue function templates ---------------------------------------

 #if BINON_CONCEPTS
	template<TValueType Val, DictType Dict, typename Key>
		auto CtnrTValue(
			Dict& dict, const Key& key, bool autoAlloc
		) -> Val&
	{
		auto& map = dict.value();
		auto keyObj{MakeObj(key)};
		if(autoAlloc) {
			auto pPair = map.find(keyObj);
			if(pPair == map.end()) {
				pPair = map.insert(*pPair, {keyObj, TValObj<Val>()});
			}
			return ObjTValue<Val>(pPair->second);
		}
		else {
			return ObjTValue<Val>(map.at(keyObj));
		}
	}
	template<TValueType Val, DictType Dict, typename Key>
		auto CtnrTValue(const Dict& dict, const Key& key) -> const Val&
	{
		auto& map = dict.value();
		return ObjTValue<Val>(map.at(MakeObj(key)));
	}
	template<TValueType Val, DictType Dict, typename Key>
		auto CtnrTValue(
			Dict&& dict, const Key& key
		) -> Val
	{
		auto& map = dict.value();
		return ObjTValue<Val>(std::move(map.at(MakeObj(key))));
	}
 #else
	template<typename Val, typename Key, typename Dict>
		auto CtnrTValue(
			Dict& dict, const Key& key, bool autoAlloc = false
		)
		-> std::enable_if_t<
			kIsTValue<Val> && kIsDictType<Dict>,
			Val&
			>
	{
		auto& map = dict.value();
		auto keyObj{MakeObj(key)};
		if(autoAlloc) {
			auto pPair = map.find(keyObj);
			if(pPair == map.end()) {
				pPair = map.insert(*pPair, {keyObj, TValObj<Val>()});
			}
			return ObjTValue<Val>(pPair->second);
		}
		else {
			return ObjTValue<Val>(map.at(keyObj));
		}
	}
	template<typename Val, typename Key, typename Dict>
		auto CtnrTValue(
			const Dict& dict, const Key& key
		)
		-> std::enable_if_t<
			kIsTValue<Val> && kIsDictType<Dict>,
			const Val&
			>
	{
		auto& map = dict.value();
		return ObjTValue<Val>(map.at(MakeObj(key)));
	}
	template<typename Val, typename Dict, typename Key>
		auto CtnrTValue(
			Dict&& dict, const Key& key
		)
		-> std::enable_if_t<
			kIsTValue<Val> && kIsDictType<Dict>,
			Val
			>
	{
		auto& map = dict.value();
		return ObjTValue<Val>(std::move(map.at(MakeObj(key))));
	}
 #endif

	//---- SetCtnrVal function templates ---------------------------------------

 #if BINON_CONCEPTS
	template<DictType Dict, typename Key, typename Val>
		auto SetCtnrVal(Dict& dict, const Key& key, const Val& val) -> Dict&
 #else
	template<typename Dict, typename Key, typename Val>
		auto SetCtnrVal(Dict& dict, const Key& key, const Val& val)
		-> std::enable_if_t<kIsDictType<Dict>, Dict&>
 #endif
	{
		dict.value().insert_or_assign(MakeObj(key), MakeObj(val));
		return dict;
	}

	//---- DelKey function templates -------------------------------------------

 #if BINON_CONCEPTS
	auto DelKey(DictType auto& dict, const auto& key) -> bool
 #else
	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, const Key& key)
		-> std::enable_if_t<kIsDictType<Dict>, bool>
 #endif
	{
		auto& map = dict.value();
		auto iter = map.find(MakeObj(key));
		if(iter == map.end()) {
			return false;
		}
		map.erase(iter);
		return true;
	}

	//---- Make... function templates ------------------------------------------

	template<typename... Pairs>
		auto MakeDictObj(Pairs&&... pairs) -> DictObj
	{
		DictObj dict;
		dict.value().reserve(sizeof...(Pairs));
		(	SetCtnrVal(
				dict, std::forward<Pairs>(pairs).first, std::forward<Pairs>(pairs).second
			),
			...
		);
		return dict;
	}
}

#endif
