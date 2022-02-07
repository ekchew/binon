#ifndef BINON_DICTHELPERS__HPP
#define BINON_DICTHELPERS__HPP

#include "objhelpers.hpp"

namespace binon {

	template<typename T>
		constexpr bool kIsDictType = std::is_base_of_v<DictBase, T>;
 BINON_IF_CONCEPTS(
	template<typename T>
		concept DictType = kIsDictType<T>;
 )

	constexpr bool kAutoAlloc = true;

	//==== Template Implementation =============================================

	//---- HasObjKey function templates ----------------------------------------

#if BINON_CONCEPTS
	auto HasObjKey(const DictType auto& dict, const BinONObj& key) -> bool
	{
		auto& map = dict.value();
		return map.find(key) != map.end();
	}
#else
	template<typename Dict, typename T>
		auto HasObjKey(const Dict& dict, const BinONObj& key)
		-> std::enable_if<kIsDictType<Dict>, bool>
	{
		auto& map = dict.value();
		return map.find(key) != map.end();
	}
#endif

	//---- HasKey function templates -------------------------------------------

 #if BINON_CONCEPTS
	auto HasKey(const DictType auto& dict, const char* key) -> bool {
		auto& map = dict.value();
		return map.find(MakeObj(key)) != map.end();
	}
	auto HasKey(const DictType auto& dict, const NonCStr auto& key) -> bool {
		auto& map = dict.value();
		return map.find(MakeObj(key)) != map.end();
	}
 #else
	template<typename Dict>
		auto HasKey(const Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict>, bool
				>
	{
		auto& map = dict.value();
		return map.find(MakeObj(key)) != map.end();
	}
	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsCStr<Key>, bool
				>
	{
		auto& map = dict.value();
		return map.find(MakeObj(key)) != map.end();
	}
 #endif

	//---- CtnrTValue function templates ---------------------------------------

#if BINON_CONCEPTS
	template<TValueType Val, DictType Dict>
		auto CntrTValue(
			Dict& dict, const BinONObj& key, bool autoAlloc = false
		) -> Val&
	{
		auto& map = dict.value();
		if(autoAlloc) {
			auto pPair = map.find(key);
			if(pPair == map.end()) {
				pPair = map.insert(*pPair, {key, TValObj<Val>()});
			}
			return ObjTValue<Val>(pPair->second);
		}
		else {
			return ObjTValue<Val>(map.at(key));
		}
	}
	template<TValueType Val, DictType Dict>
		auto CntrTValue(
			const Dict& dict, const BinONObj& key
		) -> const Val&
	{
		auto& map = dict.value();
		return ObjTValue<Val>(map.at(key));
	}
	template<TValueType Val, DictType Dict>
		auto CntrTValue(
			Dict&& dict, const BinONObj& key
		) -> Val
	{
		auto& map = dict.value();
		return ObjTValue<Val>(std::move(map.at(key)));
	}
#else
	template<typename Val, typename Dict>
		auto CntrTValue(
			Dict& dict, const BinONObj& key, bool autoAlloc = false
		)
		-> std::enable_if_t<
			kIsTValue<Val> && kIsDictType<Dict>,
			Val&
			>
	{
		auto& map = dict.value();
		if(autoAlloc) {
			auto pPair = map.find(key);
			if(pPair == map.end()) {
				pPair = map.insert(*pPair, {key, TValObj<Val>()});
			}
			return ObjTValue<Val>(pPair->second);
		}
		else {
			return ObjTValue<Val>(map.at(key));
		}
	}
	template<typename Val, typename Dict>
		auto CntrTValue(
			const Dict& dict, const BinONObj& key
		)
		-> std::enable_if_t<
			kIsTValue<Val> && kIsDictType<Dict>,
			const Val&
			>
	{
		auto& map = dict.value();
		return ObjTValue<Val>(map.at(key));
	}
	template<typename Val, typename Dict, typename Key>
		auto CntrTValue(
			Dict&& dict, const BinONObj& key
		)
		-> std::enable_if_t<
			kIsTValue<Val> && kIsDictType<Dict>,
			Val
			>
	{
		auto& map = dict.value();
		return ObjTValue<Val>(std::move(map.at(key)));
	}
#endif

	//---- GetCtnrVal function templates ---------------------------------------

 #if BINON_CONCEPTS
	template<typename Val, DictType Dict>
		auto GetCtnrVal(const Dict& dict, const char* key)
			-> TGetObjVal<Val>
	{
		return GetObjVal<Val>(dict.value().at(MakeObj(key)));
	}
	template<typename Val, DictType Dict, NonCStr Key>
		auto GetCtnrVal(const Dict& dict, const Key& key)
			-> TGetObjVal<Val>
	{
		return GetObjVal<Val>(dict.value().at(MakeObj(key)));
	}
  #else
	template<typename Val, typename Dict>
		auto GetCtnrVal(const Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict>,
				TGetObjVal<Val>
				>
	{
		return GetObjVal<Val>(dict.value().at(MakeObj(key)));
	}
	template<typename Val, typename Dict, typename Key>
		auto GetCtnrVal(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsCStr<Key>,
				TGetObjVal<Val>
				>
	{
		return GetObjVal<Val>(dict.value().at(MakeObj(key)));
	}
 #endif

	//---- SetCtnrVal function templates ---------------------------------------

 #if BINON_CONCEPTS
	auto& SetCtnrVal(DictType auto& dict, const char* key, const char* val) {
		dict.value().insert_or_assign(MakeObj(key), MakeObj(val));
		return dict;
	}
	auto& SetCtnrVal(DictType auto& dict, const char* key, const NonCStr auto& val)
	{
		dict.value().insert_or_assign(MakeObj(key), MakeObj(val));
		return dict;
	}
	auto& SetCtnrVal(DictType auto& dict, const NonCStr auto& key, const char* val)
	{
		dict.value().insert_or_assign(MakeObj(key), MakeObj(val));
		return dict;
	}
	auto& SetCtnrVal(
		DictType auto& dict, const NonCStr auto& key, const NonCStr auto& val
	) {
		dict.value().insert_or_assign(MakeObj(key), MakeObj(val));
		return dict;
	}
	template<DictType Dict, TValueType Val>
		auto& SetCtnrVal(Dict& dict, const char* key, Val&& val)
	{
		dict.value().insert_or_assign(
			MakeObj(key), MakeObj(std::forward<Val>(val))
			);
		return dict;
	}
	template<DictType Dict, NonCStr Key, TValueType Val>
		auto& SetCtnrVal(Dict& dict, const Key& key, Val&& val)
	{
		dict.value().insert_or_assign(
			MakeObj(key),
			MakeObj(std::forward<Val>(val))
			);
		return dict;
	}
 #else
	template<
		typename Dict,
		typename std::enable_if_t<kIsDictType<Dict>, int> = 0
		>
		auto& SetCtnrVal(Dict& dict, const char* key, const char* val)
	{
		dict.value().insert_or_assign(MakeObj(key), MakeObj(val));
		return dict;
	}
	template<
		typename Dict, typename Val,
		typename std::enable_if_t<kIsDictType<Dict> && !kIsCStr<Val>, int> = 0
		>
		auto& SetCtnrVal(Dict& dict, const char* key, const Val& val)
	{
		dict.value().insert_or_assign(MakeObj(key), MakeObj(val));
		return dict;
	}
	template<
		typename Dict, typename Key,
		typename std::enable_if_t<kIsDictType<Dict> && !kIsCStr<Key>, int> = 0
		>
		auto& SetCtnrVal(Dict& dict, const Key& key, const char* val)
	{
		dict.value().insert_or_assign(MakeObj(key), MakeObj(val));
		return dict;
	}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && !kIsCStr<Key> && !kIsCStr<Val>, int
			> = 0
		>
		auto& SetCtnrVal(Dict& dict, const Key& key, const Val& val)
	{
		dict.value().insert_or_assign(MakeObj(key), MakeObj(val));
		return dict;
	}
	template<
		typename Dict, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsTValue<Val>, int
			> = 0
		>
		auto& SetCtnrVal(Dict& dict, const char* key, Val&& val)
	{
		dict.value().insert_or_assign(
			MakeObj(key), MakeObj(std::forward<Val>(val))
			);
		return dict;
	}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsTValue<Val> && !kIsCStr<Key>,
			int
			> = 0
		>
		auto& SetCtnrVal(Dict& dict, const Key& key, Val&& val)
	{
		dict.value().insert_or_assign(
			MakeObj(key),
			MakeObj(std::forward<Val>(val))
			);
		return dict;
	}
 #endif

	//---- DelKey function templates -------------------------------------------

 #if BINON_CONCEPTS
	auto DelKey(DictType auto& dict, const char* key) -> bool {
		auto& map = dict.value();
		auto iter = map.find(MakeObj(key));
		if(iter == map.end()) {
			return false;
		}
		map.erase(iter);
		return true;
	}
	auto DelKey(DictType auto& dict, const NonCStr auto& key) -> bool {
		auto& map = dict.value();
		auto iter = map.find(MakeObj(key));
		if(iter == map.end()) {
			return false;
		}
		map.erase(iter);
		return true;
	}
	template<DictType Dict, TValueType Key>
		auto DelKey(Dict& dict, Key&& key) -> bool
	{
		auto& map = dict.value();
		auto iter = map.find(MakeObj(std::forward<Key>(key)));
		if(iter == map.end()) {
			return false;
		}
		map.erase(iter);
		return true;
	}
#else
	template<typename Dict>
		auto DelKey(Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict>, bool
				>
	{
		auto& map = dict.value();
		auto iter = map.find(MakeObj(key));
		if(iter == map.end()) {
			return false;
		}
		map.erase(iter);
		return true;
	}
	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsCStr<Key>, bool
				>
	{
		auto& map = dict.value();
		auto iter = map.find(MakeObj(key));
		if(iter == map.end()) {
			return false;
		}
		map.erase(iter);
		return true;
	}
	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsTValue<Key>, bool
				>
	{
		auto& map = dict.value();
		auto iter = map.find(MakeObj(std::forward<Key>(key)));
		if(iter == map.end()) {
			return false;
		}
		map.erase(iter);
		return true;
	}
 #endif

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
