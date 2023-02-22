#ifndef BINON_DICTHELPERS__HPP
#define BINON_DICTHELPERS__HPP

#include "objhelpers.hpp"
#include <initializer_list>

//	dicthelpers.hpp is structured a lot like listhelpers.hpp, so the
//	documentation here will mainly highlight differences.

namespace binon {

	template<typename T>
		constexpr bool kIsDictType = std::is_base_of_v<DictBase, T>;
 BINON_IF_CONCEPTS(
	template<typename T>
		concept DictType = kIsDictType<T>;
 )

	//	The Make... functions for dictionaries take initializer lists of
	//	key-value pairs. This allows you to use nested brace notation along the
	//	lines SDict(kStrObjCode, kUIntCode, {{"foo", 1}, {"bar", 2}})
	using TCTypePair = std::pair<ObjWrapper,ObjWrapper>;
	auto MakeDictObj(std::initializer_list<TCTypePair> pairs) -> DictObj;
	auto MakeSKDict(
		CodeByte keyCode, std::initializer_list<TCTypePair> pairs
	) -> SKDict;
	auto MakeSDict(
		CodeByte keyCode, CodeByte valCode,
		std::initializer_list<TCTypePair> pairs
	) -> SDict;

	//	FindObj() attempts to locate the key you provide in the dictionary. If
	//	it finds it, it returns a reference to the associated BinONObj in the
	//	optional return argument. Otherwise, it return std::nullopt.
	//
	//	After successfully finding the object, you can then call GetObjVal() or
	//	ObjTValue() to further take it apart (see objhelpers.hpp).
	//
	//	You can also call FindObj() simply to check if a key exists, since an
	//	OptRef evaluates true or false in say an if statement.
	//
	//auto FindObj(const DictType auto& dict, const TCType auto& key)
	//	-> OptRef<const BinONObj>;
	//auto FindObj(DictType auto& dict, const TCType auto& key)
	//	-> OptRef<BinONObj>;

	//	Note that aside from taking a key rather than an index, the dictionary
	//	version of GetCtnrVal() differs from the list counterpart in one other
	//	way. For a non-const dictionary argument, there is an option to
	//	auto-allocate. If you pass kAutoAlloc (true) as the 3rd argument, it
	//	should create a new element in the dictionary if one does not exist for
	//	your key. This element will have a default-constructed value. Without
	//	auto-allocation, GetCtnrVal() will throw std::out_of_range instead on a
	//	missing key.
	//
	//template<TCType Val, DictType Dict, TCType Key>
	//	auto GetCtnrVal(Dict& dict, const Key& key, bool autoAlloc = false)
	//	-> TGetObjVal<Val>;
	//template<TCType Val, DictType Dict, TCType Key>
	//	auto GetCtnrVal(const Dict& dict, const Key& key)
	//	-> TGetObjVal<Val>;

	//TValueType auto& CtnrTValue(
	//		DictType auto& dict, const TCType auto& key, bool autoAlloc = false
	//	);
	//const TValueType auto& CtnrTValue(
	//		const DictType auto& dict, const TCType auto& key
	//	);
	//TValueType auto CtnrTValue(
	//		DictType auto&& dict, const TCType auto& key
	//	);

	//template<DictType Dict, TCType Key, TCType Val>
	//	auto SetCtnrVal(Dict& dict, const Key& key, const Val& val) -> Dict&;

	//	DelKey() deletes the element associated with the key you provide if it
	//	exists in the dictionary. It returns true if the key existed and was
	//	removed or false it there was no such key.
	//
	//auto DelKey(DictType auto& dict, const TCType auto& key) -> bool;

	constexpr bool kAutoAlloc = true;

	//==== Template Implementation =============================================

	namespace details {
		template<typename Dict, typename Key>
			auto MakeKeyObj(const Dict& dict, const Key& key) -> BinONObj
		{
			using std::is_base_of_v;
			if constexpr(std::is_same_v<BinONObj,Key>) {
				return key;
			}
			else if constexpr(
				is_base_of_v<SKDict,Dict> ||
				is_base_of_v<SDict,Dict>
			) {
				return MakeTypeCodeObj(dict.mKeyCode, key);
			}
			else {
				return MakeObj(key);
			}
		}
		template<typename Dict, typename Val>
			auto MakeValObj(const Dict& dict, const Val& val) -> BinONObj
		{
			using std::is_base_of_v;
			if constexpr(std::is_same_v<BinONObj,Val>) {
				return val;
			}
			else if constexpr(is_base_of_v<SDict,Dict>) {
				return MakeTypeCodeObj(dict.mValCode, val);
			}
			else {
				return MakeObj(val);
			}
		}
	}

	//---- FindObj function templates ------------------------------------------

	template<typename Dict, typename Key>
		auto FindObj(const Dict& dict, const Key& key)
		BINON_CONCEPTS_FN(
			DictType<Dict> && TCType<Key>,
			kIsDictType<Dict> && kIsTCType<Key>,
			OptRef<const BinONObj>
		)
	{
		auto keyObj = details::MakeKeyObj(dict, key);
		auto& map = dict.value();
		auto iter = map.find(keyObj);
		if(iter != map.end()) {
			return iter->second;
		}
		return std::nullopt;
	}
	template<typename Dict, typename Key>
		auto FindObj(Dict& dict, const Key& key)
		BINON_CONCEPTS_FN(
			DictType<Dict> && TCType<Key>,
			kIsDictType<Dict> && kIsTCType<Key>,
			OptRef<BinONObj>
		)
	{
		auto keyObj = details::MakeKeyObj(dict, key);
		auto& map = dict.value();
		auto iter = map.find(keyObj);
		if(iter != map.end()) {
			return iter->second;
		}
		return std::nullopt;
	}

 	//---- GetCtnrVal function templates ---------------------------------------

 	template<typename Val, typename Dict, typename Key>
		auto GetCtnrVal(Dict& dict, const Key& key, bool autoAlloc)
		BINON_CONCEPTS_FN(
			TCType<Val> && DictType<Dict> && TCType<Key>,
			kIsTCType<Val> && kIsDictType<Dict> && kIsTCType<Key>,
			TGetObjVal<Val>
		)
	{
		if(!autoAlloc) {
			return GetCtnrVal(static_cast<const Dict&>(dict), key);
		}
		auto optObj = FindObj(dict, key);
		if(optObj) {
			return *optObj;
		}
		SetCtnrVal(dict, key, Val());
		return TGetObjVal<Val>();
	}
	template<typename Val, typename Dict, typename Key>
		auto GetCtnrVal(const Dict& dict, const Key& key)
		BINON_CONCEPTS_FN(
			TCType<Val> && DictType<Dict> && TCType<Key>,
			kIsTCType<Val> && kIsDictType<Dict> && kIsTCType<Key>,
			TGetObjVal<Val>
		)
	{
		return GetObjVal<Val>(dict.value().at(details::MakeKeyObj(dict, key)));
	}

	//---- CtnrTValue function templates ---------------------------------------

	template<typename Val, typename Key, typename Dict>
		auto CtnrTValue(Dict& dict, const Key& key, bool autoAlloc)
		BINON_CONCEPTS_FN(
			TValueType<Val> && DictType<Dict> && TCType<Key>,
			kIsTValue<Val> && kIsDictType<Dict> && kIsTCType<Key>,
			Val&
		)
	{
		auto& map = dict.value();
		auto keyObj{details::MakeKeyObj(dict, key)};
		if(autoAlloc) {
			auto iter = map.find(keyObj);
			if(iter == map.end()) {
				iter = map.insert(
					*iter,
					{keyObj, MakeValObj(dict, Val())}
				);
			}
			return ObjTValue<Val>(iter->second);
		}
		else {
			return ObjTValue<Val>(map.at(keyObj));
		}
	}
	template<typename Val, typename Key, typename Dict>
		auto CtnrTValue(
			const Dict& dict, const Key& key
		)
		BINON_CONCEPTS_FN(
			TValueType<Val> && DictType<Dict> && TCType<Key>,
			kIsTValue<Val> && kIsDictType<Dict> && kIsTCType<Key>,
			const Val&
		)
	{
		auto& map = dict.value();
		return ObjTValue<Val>(map.at(details::MakeKeyObj(dict, key)));
	}
	template<typename Val, typename Dict, typename Key>
		auto CtnrTValue(
			Dict&& dict, const Key& key
		)
		BINON_CONCEPTS_FN(
			TValueType<Val> && DictType<Dict> && TCType<Key>,
			kIsTValue<Val> && kIsDictType<Dict> && kIsTCType<Key>,
			Val
		)
	{
		auto& map = dict.value();
		return ObjTValue<Val>(
			std::move(map.at(details::MakeKeyObj(dict, key)))
		);
	}

	//---- SetCtnrVal function templates ---------------------------------------

	template<typename Dict, typename Key, typename Val>
		auto SetCtnrVal(Dict& dict, const Key& key, const Val& val)
		BINON_CONCEPTS_FN(
			DictType<Dict> && TCType<Key> && TCType<Val>,
			kIsDictType<Dict> && kIsTCType<Key> && kIsTCType<Val>,
			Dict&
		)
	{
		dict.value().insert_or_assign(
			details::MakeKeyObj(dict, key),
			details::MakeValObj(dict, val)
		);
		return dict;
	}

	//---- DelKey function templates -------------------------------------------

 	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, const Key& key)
		BINON_CONCEPTS_FN(
			DictType<Dict> && TCType<Key>,
			kIsDictType<Dict> && kIsTCType<Key>,
			bool
		)
	{
		auto& map = dict.value();
		auto iter = map.find(details::MakeKeyObj(dict, key));
		if(iter == map.end()) {
			return false;
		}
		map.erase(iter);
		return true;
	}
}

#endif
