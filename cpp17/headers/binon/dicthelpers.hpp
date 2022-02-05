#ifndef BINON_DICTHELPERS__HPP
#define BINON_DICTHELPERS__HPP

#include "objhelpers.hpp"

namespace binon {

	template<typename T>
		constexpr bool kIsDictType = std::is_base_of_v<DictBase, T>;

	/*
	These functions help you work with DictObj, SKDict, and SDict instances:

	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, Key key) -> bool;
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict dict, Key key) -> TBinONObjVal<Val>;
	template<typename Dict, typename Key, typename Val>
		auto SetVal(Dict& dict, Key key, Val val) -> Dict&;
	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, Key key) -> bool;

	As with the list helpers, the dict helpers let you work with more natural
	types along the lines:

		SetVal(myDict, "answer", 42);
		int i = GetVal<int>(myDict, "answer");

	DelKey() returns true if the key entry exists and the key-value pair gets
	deleted. If there is no such entry, it does nothing and returns false.

	Again, there are numerous overloads of these methods for dealing with
	move semantics and such.
	*/

	//==== Template Implementation =============================================

	//---- HasKey function templates -------------------------------------------

	template<typename Dict>
		auto HasKey(const Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict>, bool
				>
		{
			auto& map = dict.value();
			return map.find(MakeBinONObj(key)) != map.end();
		}
	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsCStr<Key>, bool
				>
		{
			auto& map = dict.value();
			return map.find(MakeBinONObj(key)) != map.end();
		}
	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key> && !kIsCStr<Key>, bool
				>
		{
			auto& map = dict.value();
			return map.find(MakeBinONObj(std::forward<Key>(key))) != map.end();
		}

	//---- GetVal function templates -------------------------------------------

	template<typename Val, typename Dict>
		auto GetVal(Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val>,
				TBinONObjVal<Val>&
				>
		{
			return BinONObjVal<Val>(dict.value().at(MakeBinONObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val> && !kIsCStr<Key>,
				TBinONObjVal<Val>&
				>
		{
			return BinONObjVal<Val>(dict.value().at(MakeBinONObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key>
					&& kIsBinONVal<Val> && !kIsCStr<Key>,
				TBinONObjVal<Val>&
				>
		{
			return BinONObjVal<Val>(
				dict.value().at(MakeBinONObj(std::forward<Key>(key)))
				);
		}
	template<typename Val, typename Dict>
		auto GetVal(const Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val>,
				const TBinONObjVal<Val>&
				>
		{
			return BinONObjVal<Val>(dict.value().at(MakeBinONObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val> && !kIsCStr<Key>,
				const TBinONObjVal<Val>&
				>
		{
			return BinONObjVal<Val>(dict.value().at(MakeBinONObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key>
					&& kIsBinONVal<Val> && !kIsCStr<Key>,
				const TBinONObjVal<Val>&
				>
		{
			return BinONObjVal<Val>(
				dict.value().at(MakeBinONObj(std::forward<Key>(key)))
				);
		}
	template<typename Val, typename Dict>
		auto GetVal(Dict&& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val>,
				TBinONObjVal<Val>
				>
		{
			return BinONObjVal<Val>(
				std::forward<Dict>(dict).value().at(MakeBinONObj(key))
				);
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict&& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val> && !kIsCStr<Key>,
				TBinONObjVal<Val>
				>
		{
			return BinONObjVal<Val>(
				std::forward<Dict>(dict).value().at(MakeBinONObj(key))
				);
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict&& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key>
					&& kIsBinONVal<Val> && !kIsCStr<Key>,
				TBinONObjVal<Val>
				>
		{
			return BinONObjVal<Val>(
				std::forward<Dict>(dict).value().at(
					MakeBinONObj(std::forward<Key>(key))
					)
				);
		}
	template<typename Val, typename Dict>
		auto GetVal(const Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsBinONVal<Val>,
				TBinONObjVal<Val>
				>
		{
			return BinONObjVal<Val>(dict.value().at(MakeBinONObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsBinONVal<Val> && !kIsCStr<Key>,
				TBinONObjVal<Val>
				>
		{
			return BinONObjVal<Val>(dict.value().at(MakeBinONObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key>
					&& !kIsBinONVal<Val> && !kIsCStr<Key>,
				TBinONObjVal<Val>
				>
		{
			return BinONObjVal<Val>(
				dict.value().at(MakeBinONObj(std::forward<Key>(key)))
				);
		}

	//---- SetVal function templates -------------------------------------------

	template<
		typename Dict,
		typename std::enable_if_t<kIsDictType<Dict>, int> = 0
		>
		auto& SetVal(Dict& dict, const char* key, const char* val) {
			dict.value().insert_or_assign(MakeBinONObj(key), MakeBinONObj(val));
			return dict;
		}
	template<
		typename Dict, typename Val,
		typename std::enable_if_t<kIsDictType<Dict> && !kIsCStr<Val>, int> = 0
		>
		auto& SetVal(Dict& dict, const char* key, const Val& val) {
			dict.value().insert_or_assign(MakeBinONObj(key), MakeBinONObj(val));
			return dict;
		}
	template<
		typename Dict, typename Key,
		typename std::enable_if_t<kIsDictType<Dict> && !kIsCStr<Key>, int> = 0
		>
		auto& SetVal(Dict& dict, const Key& key, const char* val) {
			dict.value().insert_or_assign(MakeBinONObj(key), MakeBinONObj(val));
			return dict;
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && !kIsCStr<Key> && !kIsCStr<Val>, int
			> = 0
		>
		auto& SetVal(Dict& dict, const Key& key, const Val& val) {
			dict.value().insert_or_assign(MakeBinONObj(key), MakeBinONObj(val));
			return dict;
		}
	template<
		typename Dict, typename Key,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Key> && !kIsCStr<Key>, int
			> = 0
		>
		auto& SetVal(Dict& dict, Key&& key, const char* val) {
			dict.value().insert_or_assign(
				MakeBinONObj(std::forward<Key>(key)), MakeBinONObj(val)
				);
			return dict;
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Key>
				&& !kIsCStr<Key> && !kIsCStr<Val>,
			int
			> = 0
		>
		auto& SetVal(Dict& dict, Key&& key, const Val& val) {
			dict.value().insert_or_assign(
				MakeBinONObj(std::forward<Key>(key)), MakeBinONObj(val)
				);
			return dict;
		}
	template<
		typename Dict, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Val> && !kIsCStr<Val>, int
			> = 0
		>
		auto& SetVal(Dict& dict, const char* key, Val&& val) {
			dict.value().insert_or_assign(
				MakeBinONObj(key), MakeBinONObj(std::forward<Val>(val))
				);
			return dict;
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Val>
				&& !kIsCStr<Key> && !kIsCStr<Val>,
			int
			> = 0
		>
		auto& SetVal(Dict& dict, const Key& key, Val&& val) {
			dict.value().insert_or_assign(
				MakeBinONObj(key),
				MakeBinONObj(std::forward<Val>(val))
				);
			return dict;
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Key> && kIsBinONVal<Val>
				 && !kIsCStr<Key> && !kIsCStr<Val>,
			int
			> = 0
		>
		auto& SetVal(Dict& dict, Key&& key, Val&& val) {
			dict.value().insert_or_assign(
				MakeBinONObj(std::forward<Key>(key)),
				MakeBinONObj(std::forward<Val>(val))
				);
			return dict;
		}

	//---- DelKey function templates -------------------------------------------

	template<typename Dict>
		auto DelKey(Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict>, bool
				>
		{
			auto& map = dict.value();
			auto iter = map.find(MakeBinONObj(key));
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
			auto iter = map.find(MakeBinONObj(key));
			if(iter == map.end()) {
				return false;
			}
			map.erase(iter);
			return true;
		}
	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key> && !kIsCStr<Key>, bool
				>
		{
			auto& map = dict.value();
			auto iter = map.find(MakeBinONObj(std::forward<Key>(key)));
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
		(	SetVal(
				dict, std::forward<Pairs>(pairs).first, std::forward<Pairs>(pairs).second
			),
			...
		);
		return dict;
	}
}

#endif
