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

	//	HasKey() builds a key object around the value you supply by calling
	//	MakeObj() (see objhelpers.hpp) and then checks if such a key exists in
	//	the dictionary.
	auto HasKey(const DictType auto& dict, const auto& key) -> bool;

	auto FindCtnrObj(DictType auto& dict, const auto& key) -> OptRef<BinONObj>;

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

	//	The main thing to note about CtnrTValue (besides the autoAlloc feature
	//	it shares with GetCtnrVal) is that you need to package the look-up key
	//	into a BinONObj yourself. (This is because CtnrTValue is a lower-level,
	//	higher-performance alternative to GetCtnrVal that tries to minimize
	//	internal overhead.)
	template<TValueType Val, DictType Dict>
		auto CtnrTValue(
			Dict& dict, const BinONObj& key, bool autoAlloc = false
		) -> Val&;
	template<TValueType Val, DictType Dict>
		auto CtnrTValue(const Dict& dict, const BinONObj& key) -> const Val&;
	template<TValueType Val, DictType Dict>
		auto CtnrTValue(Dict&& dict, const BinONObj& key) -> Val;

	template<DictType Dict, typename Key, typename Val>
		auto SetCtnrVal(Dict& dict, const Key& key, const Val& val) -> Dict&;

	//	DelKey() functions much like HasKey() but removes the associated element
	//	if it finds it. The return value is true if the key is found and false
	//	if not.
	auto DelKey(DictType auto& dict, const auto& key) -> bool;

 #endif

	constexpr bool kAutoAlloc = true;

	//==== Template Implementation =============================================

	//---- HasKey function templates -------------------------------------------

 #if BINON_CONCEPTS
	auto HasKey(const DictType auto& dict, const auto& key) -> bool
 #else
	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, const Key& key)
		-> std::enable_if_t<kIsDictType<Dict>, bool>
 #endif
	{
		auto& map = dict.value();
		return map.find(MakeObj(key)) != map.end();
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

	//---- FindObj function templates ------------------------------------------

#if BINON_CONCEPTS
	template<DictType Dict, typename Key>
		auto FindCtnrObj(Dict& dict, const Key& key) -> OptRef<BinONObj>
#else
	template<typename Dict, typename Key>
		auto FindCtnrObj(Dict& dict, const Key& key)
		-> std::enable_if_t<kIsDictType<Dict>, OptRef<BinONObj>>
#endif
	{
		auto& map = dict.value();
		BinONObj keyObj = MakeObj(key);
		auto pPair = map.find(keyObj);
		if(pPair == map.end()) {
			return std::nullopt;
		}
		return pPair->second;
	}

	//---- CtnrTValue function templates ---------------------------------------

 #if BINON_CONCEPTS
	template<TValueType Val, DictType Dict>
		auto CtnrTValue(
			Dict& dict, const BinONObj& key, bool autoAlloc
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
		auto CtnrTValue(const Dict& dict, const BinONObj& key) -> const Val&
	{
		auto& map = dict.value();
		return ObjTValue<Val>(map.at(key));
	}
	template<TValueType Val, DictType Dict>
		auto CtnrTValue(
			Dict&& dict, const BinONObj& key
		) -> Val
	{
		auto& map = dict.value();
		return ObjTValue<Val>(std::move(map.at(key)));
	}
 #else
	template<typename Val, typename Dict>
		auto CtnrTValue(
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
		auto CtnrTValue(
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
		auto CtnrTValue(
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
