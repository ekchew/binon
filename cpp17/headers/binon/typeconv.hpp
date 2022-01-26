#ifndef BINON_TYPECONV_HPP
#define BINON_TYPECONV_HPP

#include "varobj.hpp"

#include <sstream>

namespace binon {
	
	/*
	Some nifty code off the Internet that determines if a given type is among
	the possible member types of a std::variant.
	*/
	template<typename T, typename Variant> struct IsVariantMember;
	template<typename T, typename... EveryT>
		struct IsVariantMember<T, std::variant<EveryT...>>:
			std::disjunction<std::is_same<T, EveryT>...>
		{
		};
	template<typename T, typename Variant>
		constexpr bool kIsVariantMember = IsVariantMember<T,Variant>::value;

	/*
	This uses the above to tell you if a give type T is one of the BinON object
	types like TIntObj, TStrObj, etc.
	*/
	template<typename T>
		constexpr bool kIsBinONObj
			= kIsVariantMember<std::decay_t<T>,TVarBase>;

	/*
	The TypeConv struct is used by the MakeVarObj and VarObjVal methods below to
	map common types onto BinON object types for easy conversion between the
	two. The base definition signifies failure to find a mapping, but the
	template is specialized in many ways to map the various types.

		General Type         BinON Obj    Notes
		____________         _________    _____

		bool                 TBoolObj     native value type
		TBoolObj             TBoolObj
		int8_t               TIntObj
		int16_t              TIntObj
		int32_t              TIntObj
		int64_t              TIntObj
		TIntVal              TIntObj      native value type
		TIntObj              TIntObj
		uint8_t              TUIntObj
		uint16_t             TUIntObj
		uint32_t             TUIntObj
		uint64_t             TUIntObj
		TUIntVal             TIntObj      native value type
		TUIntObj             TUIntObj
		TFloat32             TFloat32Obj  native value type, see floattypes.hpp
		TFloat32Obj          TFloat32Obj
		TFloat64             TFloatObj    native value type, see floattypes.hpp
		TFloatObj            TFloatObj
		TBufferVal           TBufferObj   native value type
		TBufferObj           TBufferObj
		std::string          TStrObj
		std::string_view     TStrObj
		HyStr                TStrObj      native value type, see hystr.hpp
		const char*          TStrObj      VarObjVal() returns std::string_view
		TStringObj           TStrObj
		std::vector<TVarObj> TListObj     native value type
		TListObj             TListObj
		TSList               TSList
		std::unordered_map<TVarObj,TVarObj>
		                     TDictObj     native value type
		TDictObj             TDictObj
		TSKDict              TSKDict
		TSDict               TSDict

	First of all, you can see that all BinON object types map onto themselves as
	you might expect. In fact, for list and dictionary subtypes like TSList, the
	only way to specify them in the functions found here is to supply the object
	type directly. That's because they all share the same value type as the base
	type. (e.g. TListObj::TValue is the same std::vector<VarObj> as
	TSList::TValue. That means technically, you could mix object types even in
	a TSList, but when you go to encode it, you will get a binon::TypeErr
	exception. So don't!)

	Those general types that are labelled "native value type" in the notes are
	the types the corresponding object uses to store its data. For example,
	TStrObj::TValue is a HyStr, and TStrObj::value() returns this type
	(ignoring const and reference qualifiers of course). From MakeVarObj's
	standpoint, that means move constructing is possible. From VarObjVal's
	standpoint, that means move returning is possible, and you can also get
	a mutable reference to the value so that you can modify it in-place.
	*/
	template<typename T, typename Enable = void>
		struct TypeConv {
			static_assert(true, "Cannot convert type to/from BinON object");
			/*
			Specializations of this class looks something like this:

			using TObj = ...
			using TVal = ...
			static auto ValTypeName() -> HyStr;
			static auto GetVal(TVarObj& obj) -> TVal&;
			static auto GetVal(const TVarObj& obj) -> TVal;
			static auto GetVal(TVarObj&& obj) -> TVal;

			You would not normally need to access TypeConv directly, since
			MakeVarObj(), VarObjVal(), etc. do that for you. One possible
			exception would be to call the ValTypeName() function. This gives
			you a string containing the type name of the value you pass it,
			which could be useful for debugging perhaps? (Note that you can
			also get the name of any BinON object class from its kClsName
			class constant.)
			*/
		};

	/*
	TVarObjVal<T> is the type returned by the VarObjVal<T>() method defined
	below (minus any const/reference qualifiers).

	In most case, TVarObjVal<T> is simply T. There are 2 notable exceptions.
	If T is a BinON object type, TVarObjVal<T> will be its value type.
	For example TVarObjVal<TBoolObj> is TBoolObj::TValue, which in turn is
	simply the bool type.

	The other exception was mentioned earlier. TVarObjVal<const char*> is
	std::string_view -- not const char*. That's because the HyStr stored as a
	TStrObj's value cannot be returned as a old school C string pointer.
	*/
	template<typename T>
		using TVarObjVal = typename TypeConv<std::decay_t<T>>::TVal;

	/*
	A type is considered BinON-moveable if it is either a primary object type or
	is the native value type of such an object. For example, TStrObj and HyStr
	are both moveable since TStrObj is one of the core BinON-object types and
	HyStr is its internal value type (in other words, TStrObj::TValue is HyStr).
	*/
	template<typename T>
		constexpr bool kBinONMoves =
			kIsBinONObj<T> || std::is_same_v<std::decay_t<T>, TVarObjVal<T>>;

	/*
	MakeVarObj() returns a TVarObj based on the type you give it, provided said
	type is known to TypeConv. (Otherwise, it will fail to compile.)
	
	Note that a move version of MakeVarObj() is available only for
	BinON-moveable types.

	template<typename T>
		auto MakeVarObj(T&& v) noexcept -> TVarObj;
	*/
	template<typename T>
		auto MakeVarObj(const T& v) -> TVarObj;

	/*
	VarObjVal() is essentially the opposite of MakeVarObj(). It extracts the
	value inside a VarObj and returns it to you as the type you specify. As with
	MakeVarObj(), it may fail a static assertion if the return type is unknown
	to TypeConv. It may also throw std::bad_variant_access if the VarObj does
	not contain the expected variant for the type you give it. For example,
	VarObjVal<int>(myVarObj) will fail if myVarObj contains a TStrObj rather
	than a TIntObj.

	As with MakeVarObj(), certain versions of VarObjVal are only available when
	you are using BinON-moveable types. These include the mutable L-value and
	R-value reference types. The constant L-value is always available, but may
	return a copy or a constant reference depending on what is appropriate.
	You can assign this to an auto&& variable if you want to avoid copying
	where possible. For example:

		const auto varObj = MakeVarObj("foo");
		auto&& s1 = VarObjVal<HyStr>(myVarObj);
		auto&& s2 = VarObjVal<std::string>(myVarObj);

	Here, s1 should be a reference to the native StrObj value inside varObj,
	while s2 will be copy of the text.

	template<typename T>
		auto VarObjVal(TVarObj& obj) -> TVarObjVal<T>&;
	template<typename T>
		auto VarObjVal(TVarObj&& obj) -> TVarObjVal<T>&&;
	*/
	template<typename T>
		auto VarObjVal(const TVarObj& obj) -> TVarObjVal<T>&&;

	//---- List object helper functions ----------------------------------------

	template<typename T>
		constexpr bool kIsListType = std::is_base_of_v<TListType, T>;

	/*
	These methods are defined (conditionally for some overloads where
	necessary) for TListObj and TSList objects:

	template<typename T, typename List>
		auto GetVal(List& list, std::size_t index) -> TVarObjVal<T>&;
	template<typename T, typename List>
		auto GetVal(const List& list, std::size_t index) -> TVarObjVal<T>&&;
	template<typename T, typename List>
		auto GetVal(List&& list, std::size_t index) -> TVarObjVal<T>&&;

	template<typename List, typename T>
		void SetVal(List& list, std::size_t index, const T& v);
	template<typename List, typename T>
		void SetVal(List& list, std::size_t index, T&& v);

	template<typename T, typename List>
		void AppendVal(List& list, const T& v);
	template<typename T, typename List>
		void AppendVal(List& list, T&& v);

	They basically let you deal with more natural types. Rather than having
	to go

		auto& elem = myListObj.value().at(0);
		int i = static_cast<int>(std::get<TIntObj>(elem).scalar());
		myListObj.value().push_back(StrObj{"foo"});

	you can go

		int i = GetVal<int>(myListObj, 0);
		AppendVal(myListObj, "foo");
	*/

	//---- Dict object helper functions ----------------------------------------

	template<typename T>
		constexpr bool kIsDictType = std::is_base_of_v<TDictType, T>;

	/*
	These methods are defined (conditionally for some overloads where
	necessary) for TDictObj, TSKDict, and TSDict objects:

	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, const Key& key) -> bool;
	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, Key&& key) -> bool;

	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict& dict, const Key& key) -> TVarObjVal<Val>&;
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict& dict, Key&& key) -> TVarObjVal<Val>&;
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, const Key& key) -> TVarObjVal<Val>&&;
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, Key&& key) -> TVarObjVal<Val>&&;
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict&& dict, const Key& key) -> TVarObjVal<Val>&&;
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict&& dict, Key&& key) -> TVarObjVal<Val>&&;

	template<typename Dict, typename Key, typename Val>
		void SetVal(Dict& dict, const Key& key, const Val& val);
	template<typename Dict, typename Key, typename Val>
		void SetVal(Dict& dict, Key&& key, const Val& val);
	template<typename Dict, typename Key, typename Val>
		void SetVal(Dict& dict, const Key& key, Val&& val);
	template<typename Dict, typename Key, typename Val>
		void SetVal(Dict& dict, Key&& key, Val&& val);

	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, const Key& key) -> bool;
	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, Key&& key) -> bool;

	As with the list helpers, the dict helpers let you work with more natural
	types along the lines:

		SetVal(myDict, "answer", 42);
		int i = GetVal<int>(myDict, "answer");

	DelKey() returns true if the key entry exists and the key-value pair gets
	deleted. If there is no such entry, it does nothing and returns false.
	*/

	//==== Template Implementation =============================================

	//---- TypeConv specializations -------------------------------------------

	template<typename T>
		struct TypeConv<T, std::reference_wrapper<T>> {
			using TObj = typename TypeConv<T>::TObj;
			using TVal = typename TypeConv<T>::TVal;
			static auto ValTypeName() -> HyStr {
					return TypeConv<T>::ValTypeName();
				}
			static auto GetVal(TVarObj& obj) -> TVal& {
					return TypeConv<T>::GetVal(obj);
				}
			static auto GetVal(const TVarObj& obj) -> TVal {
					return TypeConv<T>::GetVal(obj);
				}
			static auto GetVal(TVarObj&& obj) -> TVal {
					return TypeConv<T>::GetVal(std::forward<TVarObj>(obj));
				}
		};
	template<typename T>
		struct TypeConv<T, std::enable_if_t<kIsBinONObj<T>>> {
			using TObj = T;
			using TVal = typename TObj::TValue;
			static auto ValTypeName() -> HyStr { return TObj::kClsName; }
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(TVarObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<TVarObj>(obj)).value();
				}
		};
	template<typename T>
		struct TypeConv<
			T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>
			>
		{
			using TObj = TIntObj;
			using TVal = T;
			static auto ValTypeName() -> HyStr {
					switch(sizeof(TVal)) {
					 case 1: return "int8_t";
					 case 2: return "int16_t";
					 case 4: return "int32_t";
					 case 8: return "int64_t";
					 default: {
							std::ostringstream oss;
							oss << "SIGNED_INTEGER(" << sizeof(TVal) << ")";
							return std::move(oss).str();
						}
					}
				}
			static auto GetVal(const TVarObj& obj) -> TVal {
					return static_cast<TVal>(
						std::get<TObj>(obj).value().scalar()
						);
				}
		};
	template<typename T>
		struct TypeConv<
			T, std::enable_if_t<std::is_unsigned_v<T>>
			>
		{
			using TObj = TUIntObj;
			using TVal = T;
			static auto ValTypeName() -> HyStr {
					switch(sizeof(TVal)) {
					 case 1: return "uint8_t";
					 case 2: return "uint16_t";
					 case 4: return "uint32_t";
					 case 8: return "uint64_t";
					 default: {
							std::ostringstream oss;
							oss << "UNSIGNED_INTEGER(" << sizeof(TVal) << ")";
							return std::move(oss).str();
						}
					}
				}
			static auto GetVal(const TVarObj& obj) -> TVal {
					return static_cast<TVal>(
						std::get<TObj>(obj).value().scalar()
						);
				}
		};
	template<>
		struct TypeConv<TIntVal> {
			using TObj = TIntObj;
			using TVal = TIntVal;
			static auto ValTypeName() -> HyStr { return "TIntVal"; }
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<TUIntVal> {
			using TObj = TUIntObj;
			using TVal = TUIntVal;
			static auto ValTypeName() -> HyStr { return "TUIntVal"; }
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<bool> {
			using TObj = TBoolObj;
			using TVal = bool;
			static auto ValTypeName() -> HyStr { return "bool"; }
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<types::TFloat64> {
			using TObj = TFloatObj;
			using TVal = types::TFloat64;
			static auto ValTypeName() -> HyStr { return "TFloat64"; }
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<types::TFloat32> {
			using TObj = TFloat32Obj;
			using TVal = types::TFloat32;
			static auto ValTypeName() -> HyStr { return "TFloat32"; }
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<HyStr> {
			using TObj = TStrObj;
			using TVal = HyStr;
			static auto ValTypeName() -> HyStr { return "HyStr"; }
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(TVarObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<TVarObj>(obj)).value();
				}
		};
	template<>
		struct TypeConv<std::string> {
			using TObj = TStrObj;
			using TVal = std::string;
			static auto ValTypeName() -> HyStr { return "string"; }
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value().asStr();
				}
		};
	template<>
		struct TypeConv<std::string_view> {
			using TObj = TStrObj;
			using TVal = std::string_view;
			static auto ValTypeName() -> HyStr { return "string_view"; }
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value().asView();
				}
		};
	template<>
		struct TypeConv<char*> {
			using TObj = TStrObj;
			using TVal = std::string_view;
			static auto ValTypeName() -> HyStr { return "const char*"; }
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value().asView();
				}
		};
	template<>
		struct TypeConv<TBufferVal> {
			using TObj = TBufferObj;
			using TVal = TBufferVal;
			static auto ValTypeName() -> HyStr { return "TBufferVal"; }
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(TVarObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<TVarObj>(obj)).value();
				}
		};
	template<>
		struct TypeConv<TListObj::TValue> {
			using TObj = TListObj;
			using TVal = TListObj::TValue;
			static auto ValTypeName() -> HyStr { return "vector<TVarObj>"; }
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(TVarObj&& obj) ->TVal {
					return std::get<TObj>(std::forward<TVarObj>(obj)).value();
				}
		};
	template<>
		struct TypeConv<TDictObj::TValue> {
			using TObj = TDictObj;
			using TVal = TDictObj::TValue;
			static auto ValTypeName() -> HyStr {
					return "unordered_map<TVarObj,TVarObj>";
				}
			static auto GetVal(TVarObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const TVarObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(TVarObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<TVarObj>(obj)).value();
				}
		};

	//---- MakeVarObj ----------------------------------------------------------

	template<typename T>
		auto MakeVarObj(const T& v) -> TVarObj {
			return typename TypeConv<T>::TObj(v);
		}
	template<typename T>
		auto MakeVarObj(T&& v) noexcept
			-> std::enable_if_t<kBinONMoves<T>,TVarObj>
		{
			return typename TypeConv<T>::TObj(std::forward<T>(v));
		}

	//---- VarObjVal -----------------------------------------------------------

	template<typename T>
		auto VarObjVal(TVarObj& obj)
			-> std::enable_if_t<kBinONMoves<T>, TVarObjVal<T>&>
		{
			return TypeConv<std::decay<T>>::GetVal(obj);
		}
	template<typename T>
		auto VarObjVal(const TVarObj& obj) -> TVarObjVal<T>&& {
			return TypeConv<std::decay<T>>::GetVal(obj);
		}
	template<typename T>
		auto VarObjVal(TVarObj&& obj)
			-> std::enable_if_t<kBinONMoves<T>, TVarObjVal<T>&&>
		{
			return TypeConv<std::decay<T>>::GetVal(std::forward<TVarObj>(obj));
		}

	//---- List object helper functions ----------------------------------------

	template<typename T, typename List>
		auto GetVal(List& list, std::size_t index)
			-> std::enable_if_t<
				kIsListType<List> && kBinONMoves<T>,
				TVarObjVal<T>&
				>
		{
			return VarObjVal<T>(list.value().at(index));
		}
	template<typename T, typename List>
		auto GetVal(const List& list, std::size_t index)
			-> std::enable_if_t<kIsListType<List>, TVarObjVal<T>&&>
		{
			return VarObjVal<T>(list.value().at(index));
		}
	template<typename T, typename List>
		auto GetVal(List&& list, std::size_t index)
			-> std::enable_if_t<
				kIsListType<List> && kBinONMoves<T>,
				TVarObjVal<T>&&
				>
		{
			return VarObjVal<T>(std::forward<List>(list).value().at(index));
		}
	template<
		typename List, typename T,
		typename std::enable_if_t<kIsListType<List>, int> = 0
		>
		void SetVal(List& list, std::size_t index, const T& v) {
			list.value().at(index) = MakeVarObj(v);
		}
	template<
		typename List, typename T,
		typename std::enable_if_t<
			kIsListType<List> && kBinONMoves<T>, int
			> = 0
		>
		void SetVal(List& list, std::size_t index, T&& v) {
			list.value().at(index) = MakeVarObj(std::forward<T>(v));
		}
	template<
		typename List, typename T,
		typename std::enable_if_t<kIsListType<List>, int> = 0
		>
		void AppendVal(List& list, const T& v) {
			list.value().push_back(MakeVarObj(v));
		}
	template<
		typename List, typename T,
		typename std::enable_if_t<
			kIsListType<List> && kBinONMoves<T>, int
			> = 0
		>
		void AppendVal(List& list, T&& v) {
			list.value().push_back(MakeVarObj(std::forward<T>(v)));
		}

	//---- Dict object helper functions ----------------------------------------

	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict>, bool
				>
		{
			auto& map = dict.value();
			return map.find(MakeVarObj(key)) != map.end();
		}
	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kBinONMoves<Key>, bool
				>
		{
			auto& map = dict.value();
			return map.find(MakeVarObj(std::forward<Key>(key))) != map.end();
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kBinONMoves<Val>,
				TVarObjVal<Val>&
				>
		{
			return VarObjVal<Val>(dict.value().at(MakeVarObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kBinONMoves<Key> && kBinONMoves<Val>,
				TVarObjVal<Val>&
				>
		{
			return VarObjVal<Val>(
				dict.value().at(MakeVarObj(std::forward<Key>(key)))
				);
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict>,
				TVarObjVal<Val>&&
				>
		{
			return VarObjVal<Val>(dict.value().at(MakeVarObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kBinONMoves<Key>,
				TVarObjVal<Val>&&
				>
		{
			return VarObjVal<Val>(
				dict.value().at(MakeVarObj(std::forward<Key>(key)))
				);
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict&& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kBinONMoves<Val>,
				TVarObjVal<Val>&&
				>
		{
			return VarObjVal<Val>(
				std::forward<Dict>(dict).value().at(MakeVarObj(key))
				);
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict&& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kBinONMoves<Key> && kBinONMoves<Val>,
				TVarObjVal<Val>&&
				>
		{
			return VarObjVal<Val>(
				std::forward<Dict>(dict).value().at(
					MakeVarObj(std::forward<Key>(key))
					)
				);
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<kIsDictType<Dict>, int> = 0
		>
		void SetVal(Dict& dict, const Key& key, const Val& val) {
			dict.value().at(MakeVarObj(key)) = MakeVarObj(val);
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kBinONMoves<Key>, int
			> = 0
		>
		void SetVal(Dict& dict, Key&& key, const Val& val) {
			dict.value().at(MakeVarObj(std::forward<Key>(key)))
				= MakeVarObj(val);
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kBinONMoves<Val>, int
			> = 0
		>
		void SetVal(Dict& dict, const Key& key, Val&& val) {
			dict.value().at(MakeVarObj(key))
				= MakeVarObj(std::forward<Val>(val));
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kBinONMoves<Key> && kBinONMoves<Val>, int
			> = 0
		>
		void SetVal(Dict& dict, Key&& key, Val&& val) {
			dict.value().at(MakeVarObj(std::forward<Key>(key)))
				= MakeVarObj(std::forward<Val>(val));
		}
	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict>, bool
				>
		{
			auto& map = dict.value();
			auto iter = map.find(MakeVarObj(key));
			if(iter == map.end()) {
				return false;
			}
			map.erase(iter);
			return true;
		}
	template<typename Dict, typename Key>
		auto DelKey(Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kBinONMoves<Key>, bool
				>
		{
			auto& map = dict.value();
			auto iter = map.find(MakeVarObj(std::forward<Key>(key)));
			if(iter == map.end()) {
				return false;
			}
			map.erase(iter);
			return true;
		}
}

#endif
