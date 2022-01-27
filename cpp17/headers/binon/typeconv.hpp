#ifndef BINON_TYPECONV_HPP
#define BINON_TYPECONV_HPP

#include "varobj.hpp"

#include <sstream>

namespace binon {
	/*
	kIsCStr tells you whether a given type is a C string. It's used by some of
	the template specializations used here.
	*/
	template<typename T>
		constexpr bool kIsCStr = std::is_same_v<std::decay_t<T>,const char*>;

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
	The TypeConv struct maps common types onto BinON object types for easy
	conversion between the two. You usually don't access it directly, since
	higher-level functions like MakeVarObj(), VarObjVal(), and the various
	list and dict helper functions do so for you.

	TypeConv supports the following mappings:

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
	type. For example, TListObj::TValue and TSList::TValue are the same type:
	std::vector<TVarObj>. (That means technically, you could mix object types
	even in a TSList, but when you go to encode it, you will get a
	binon::TypeErr exception. So don't!)

	Where you see "native value type" in the notes, this is the type the
	enclosing object uses to store its data. For example, TBoolObj::TValue is
	bool, and calling value() on a TBoolObj will return a bool. Native value
	types get special treatment in that they can be moved (not just copied)
	into and out of objects since there is no need to convert the data type.
	Also, getter functions (e.g. VarObjVal()) can return the value by reference
	rather than value. Provided the TVarObj in question is not a constant, you
	can even modify the value in-place.
	*/
	template<typename T, typename Enable = void>
		struct TypeConv {
			static_assert(true, "Cannot convert type to/from BinON object");
			/*
			The base definition of TypeConv is actually illegal. Finding
			yourself here means TypeConv does not recognize your type T.
			All the work is done in template specializations of this class.

			Here is a rough outline of what you would find in particular
			specialization:

			using TObj = ...
			using TVal = ...
			static auto ValTypeName() -> HyStr;
			static auto GetVal(TVarObj) -> TVal;

			TObj is the data type of the BinON object that would wrap the type
			you supply. So for example, TypeConv<bool>::TObj would be TBoolObj.

			TVal is typically your supplied type T with a few exceptions:
				- If T is a BinON object type, TVal will be its internal value
				  type. For example, TypeConv<TBoolObj>::TVal is
				  TBoolObj::TValue which, in turn, is simply bool.
				- If T is a std::reference_wrapper<U>, TVal will be U.
				- There is a special case for when T is const char*.
				  Setter functions like MakeVarObj() will treat a const char*
				  as a C string and build TStrObj variants out of them.
				  But a TStrObj cannot return a C string, so getter functions
				  like VarObjVal() return the next best thing: a string view.
				  So for T = const char*, TVal will be std::string_view.

			ValTypeName() simply returns a string naming the TVal type which
			could be useful in debugging? For example, if you make T a 32-bit
			integer, the returned type name would be HyStr{"int32_t"}. Note
			that you can also get the name of a BinON class from its kClsName
			field (e.g. TBoolObj::kClsName is HyStr{"TBoolObj"}).

			The GetVal() class method takes a TVarObj and attempts to extract
			a value of type TVal from it. Typically, you would call VarObjVal()
			instead, which would call TypeConv::GetVal() internally. Though it's
			shown in its most general form above, it is often overloaded in
			specializations to handle move semantics and returning by reference
			where applicable according to what T is. When assigning the return
			value to a variable, it's good to go like this:

				auto&& v = TypeConv<WHATEVER>::GetVal(myVarObj);

			The && make v happy to be either a reference to or copy of the
			return value, depending on what GetVal() decides to give it.
			*/
		};

	/*
	TVarObjVal<T> is the core type returned by the VarObjVal<T>() function.
	(It is equivalent to the TVal type inside TypeConv except that the type
	you pass to TVarObjVal is decayed first to remove const/reference
	qualifiers and such.)
	*/
	template<typename T>
		using TVarObjVal = typename TypeConv<std::decay_t<T>>::TVal;

	/*
	kIsBinONVal<T> evaluates true if your type T is the native value type of a
	BinON object. This would be the type that is denoted TValue in the class.
	For example, HyStr is a BinON value because it is TStrObj::TValue.

	BinON value types are useful because if you specify one in the functions
	below, you can access the value directly and even modify it in-place,
	since it's not one of these data types that need to be converted first.
	BinON value types can also be moved--not just copied--into/out of place.
	*/
	template<typename T>
		constexpr bool kIsBinONVal =
			std::is_same_v<
				std::decay_t<T>,
				typename TypeConv<std::decay_t<T>>::TObj::TValue
				>;

	/*
	MakeVarObj() returns a TVarObj based on the type you give it, provided said
	type is known to TypeConv. (Otherwise, it will fail to compile.)

	Conceptually, it looks like this:

		template<typename T>
			auto MakeVarObj(T value) -> TVarObj;

	In practice, it supports copy and--where available for the type
	(see kIsBinONVal)--move semantics on the T argument.
	*/

	/*
	VarObjVal() is essentially the opposite of MakeVarObj(). It extracts the
	value inside a TVarObj and returns it to you as the type you specify.

	So it looks something like this:

		template<typename T>
			auto VarObjVal(TVarObj varObj) -> TVarObjVal<T>;

	As with MakeVarObj(), you can use move semantics provided your type T is
	a proper BinON value type (see kIsBinONVal). For example, you could write
	things like:

		auto str = VarObjVal<HyStr>(std::move(myVarObj));
		VarObjVal<HyStr>(myVarObj) = "new string value";

	This only works with native BinON value types though. Don't try this say
	with std::string instead of HyStr since it would require a conversion.
	But you can still use copy semantics with std::string and do somthing
	like this instead:

		auto str = VarObjVal<std::string>(myVarObj);
		myVarObj = MakeVarObj<std::string>("new string value");

	That approach should work with any data type recognized by TypeConv, albeit
	with more overhead for any dynamically allocated types since there will be
	a lot of buffer allocating/copying going on.
	*/

	//---- List object helper functions ----------------------------------------

	template<typename T>
		constexpr bool kIsListType = std::is_base_of_v<TListType, T>;

	/*
	These functions help you work with TListObj and TSList instances:

		template<typename T, typename List>
			auto GetVal(List list, std::size_t index) -> TVarObjVal<T>;
		template<typename List, typename T>
			void SetVal(List& list, std::size_t index, T value);
		template<typename T, typename List>
			void AppendVal(List& list, T value);

	They basically let you deal with more natural types. Rather than having
	to go

		auto& elem = myListObj.value().at(0);
		int i = static_cast<int>(std::get<TIntObj>(elem).scalar());
		myListObj.value().push_back(StrObj{"foo"});

	you can go

		int i = GetVal<int>(myListObj, 0);
		AppendVal(myListObj, "foo");

	The functions are built atop MakeVarObj() and VarObjVal(), so much of what
	applies to them applies here as well. For example, if you pick your data
	type carefully, you can edit a list element returned by GetVal() in-place.
	*/

	//---- Dict object helper functions ----------------------------------------

	template<typename T>
		constexpr bool kIsDictType = std::is_base_of_v<TDictType, T>;

	/*
	These functions help you work with TDictObj, TSKDict, and TSDict instances:

	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, Key key) -> bool;
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict dict, Key key) -> TVarObjVal<Val>;
	template<typename Dict, typename Key, typename Val>
		void SetVal(Dict& dict, Key key, Val val);
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
			static auto GetVal(const TVarObj& obj) -> const TVal& {
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
			static auto GetVal(const TVarObj& obj) -> const TVal& {
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
			static auto GetVal(const TVarObj& obj) -> const TVal& {
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
			static auto GetVal(const TVarObj& obj) -> const TVal& {
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
			static auto GetVal(const TVarObj& obj) -> const TVal& {
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
	template<typename T>
		struct TypeConv<T, std::enable_if_t<kIsCStr<T>>> {
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
			static auto GetVal(TVarObj&& obj) -> TVal {
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

	inline auto MakeVarObj(const char* s) -> TVarObj {
		return TStrObj(s);
	}
	template<typename T>
		auto MakeVarObj(const T& v)
			-> std::enable_if_t<!kIsCStr<T>, TVarObj>
		{
			return typename TypeConv<T>::TObj(v);
		}
	template<typename T>
		auto MakeVarObj(T&& v) noexcept
			-> std::enable_if_t<kIsBinONVal<T>,TVarObj>
		{
			return typename TypeConv<T>::TObj(std::forward<T>(v));
		}

	//---- VarObjVal -----------------------------------------------------------

	template<typename T>
		auto VarObjVal(TVarObj& obj)
			-> std::enable_if_t<kIsBinONVal<T>, TVarObjVal<T>&>
		{
			return TypeConv<std::decay_t<T>>::GetVal(obj);
		}
	template<typename T>
		auto VarObjVal(const TVarObj& obj)
			-> std::enable_if_t<kIsBinONVal<T>, const TVarObjVal<T>&>
		{
			return TypeConv<std::decay_t<T>>::GetVal(obj);
		}
	template<typename T>
		auto VarObjVal(TVarObj&& obj)
			-> std::enable_if_t<kIsBinONVal<T>, TVarObjVal<T>>
		{
			return
				TypeConv<std::decay_t<T>>::GetVal(std::forward<TVarObj>(obj));
		}
	template<typename T>
		auto VarObjVal(const TVarObj& obj)
			-> std::enable_if_t<!kIsBinONVal<T>, TVarObjVal<T>> {
			return TypeConv<std::decay_t<T>>::GetVal(obj);
		}

	//---- List object helper functions ----------------------------------------

	template<typename T, typename List>
		auto GetVal(List& list, std::size_t index)
			-> std::enable_if_t<
				kIsListType<List> && kIsBinONVal<T>,
				TVarObjVal<T>&
				>
		{
			return VarObjVal<T>(list.value().at(index));
		}
	template<typename T, typename List>
		auto GetVal(const List& list, std::size_t index)
			-> std::enable_if_t<
				kIsListType<List> && kIsBinONVal<T>,
				const TVarObjVal<T>&
				>
		{
			return VarObjVal<T>(list.value().at(index));
		}
	template<typename T, typename List>
		auto GetVal(List&& list, std::size_t index)
			-> std::enable_if_t<
				kIsListType<List> && kIsBinONVal<T>,
				TVarObjVal<T>
				>
		{
			return VarObjVal<T>(std::forward<List>(list).value().at(index));
		}
	template<typename T, typename List>
		auto GetVal(const List& list, std::size_t index)
			-> std::enable_if_t<
				kIsListType<List> && !kIsBinONVal<T>,
				TVarObjVal<T>
				>
		{
			return VarObjVal<T>(list.value().at(index));
		}

	template<
		typename List,
		typename std::enable_if_t<kIsListType<List>, int> = 0
		>
		void SetVal(List& list, std::size_t index, const char* s) {
			list.value().at(index) = MakeVarObj(s);
		}
	template<
		typename List, typename T,
		typename std::enable_if_t<kIsListType<List> && !kIsCStr<T>, int> = 0
		>
		void SetVal(List& list, std::size_t index, const T& v) {
			list.value().at(index) = MakeVarObj(v);
		}
	template<
		typename List, typename T,
		typename std::enable_if_t<
			kIsListType<List> && kIsBinONVal<T> && !kIsCStr<T>, int
			> = 0
		>
		void SetVal(List& list, std::size_t index, T&& v) {
			list.value().at(index) = MakeVarObj(std::forward<T>(v));
		}

	template<
		typename List,
		typename std::enable_if_t<kIsListType<List>, int> = 0
		>
		void AppendVal(List& list, const char* s) {
			list.value().push_back(MakeVarObj(s));
		}
	template<
		typename List, typename T,
		typename std::enable_if_t<kIsListType<List> && !kIsCStr<T>, int> = 0
		>
		void AppendVal(List& list, const T& v) {
			list.value().push_back(MakeVarObj(v));
		}
	template<
		typename List, typename T,
		typename std::enable_if_t<
			kIsListType<List> && kIsBinONVal<T> && !kIsCStr<T>, int
			> = 0
		>
		void AppendVal(List& list, T&& v) {
			list.value().push_back(MakeVarObj(std::forward<T>(v)));
		}

	//---- Dict object helper functions ----------------------------------------

	template<typename Dict>
		auto HasKey(const Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict>, bool
				>
		{
			auto& map = dict.value();
			return map.find(MakeVarObj(key)) != map.end();
		}
	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsCStr<Key>, bool
				>
		{
			auto& map = dict.value();
			return map.find(MakeVarObj(key)) != map.end();
		}
	template<typename Dict, typename Key>
		auto HasKey(const Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key> && !kIsCStr<Key>, bool
				>
		{
			auto& map = dict.value();
			return map.find(MakeVarObj(std::forward<Key>(key))) != map.end();
		}

	template<typename Val, typename Dict>
		auto GetVal(Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val>,
				TVarObjVal<Val>&
				>
		{
			return VarObjVal<Val>(dict.value().at(MakeVarObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val> && !kIsCStr<Key>,
				TVarObjVal<Val>&
				>
		{
			return VarObjVal<Val>(dict.value().at(MakeVarObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key>
					&& kIsBinONVal<Val> && !kIsCStr<Key>,
				TVarObjVal<Val>&
				>
		{
			return VarObjVal<Val>(
				dict.value().at(MakeVarObj(std::forward<Key>(key)))
				);
		}
	template<typename Val, typename Dict>
		auto GetVal(const Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val>,
				const TVarObjVal<Val>&
				>
		{
			return VarObjVal<Val>(dict.value().at(MakeVarObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val> && !kIsCStr<Key>,
				const TVarObjVal<Val>&
				>
		{
			return VarObjVal<Val>(dict.value().at(MakeVarObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key>
					&& kIsBinONVal<Val> && !kIsCStr<Key>,
				const TVarObjVal<Val>&
				>
		{
			return VarObjVal<Val>(
				dict.value().at(MakeVarObj(std::forward<Key>(key)))
				);
		}
	template<typename Val, typename Dict>
		auto GetVal(Dict&& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val>,
				TVarObjVal<Val>
				>
		{
			return VarObjVal<Val>(
				std::forward<Dict>(dict).value().at(MakeVarObj(key))
				);
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict&& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Val> && !kIsCStr<Key>,
				TVarObjVal<Val>
				>
		{
			return VarObjVal<Val>(
				std::forward<Dict>(dict).value().at(MakeVarObj(key))
				);
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(Dict&& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key>
					&& kIsBinONVal<Val> && !kIsCStr<Key>,
				TVarObjVal<Val>
				>
		{
			return VarObjVal<Val>(
				std::forward<Dict>(dict).value().at(
					MakeVarObj(std::forward<Key>(key))
					)
				);
		}
	template<typename Val, typename Dict>
		auto GetVal(const Dict& dict, const char* key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsBinONVal<Val>,
				TVarObjVal<Val>
				>
		{
			return VarObjVal<Val>(dict.value().at(MakeVarObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsBinONVal<Val> && !kIsCStr<Key>,
				TVarObjVal<Val>
				>
		{
			return VarObjVal<Val>(dict.value().at(MakeVarObj(key)));
		}
	template<typename Val, typename Dict, typename Key>
		auto GetVal(const Dict& dict, Key&& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && kIsBinONVal<Key>
					&& !kIsBinONVal<Val> && !kIsCStr<Key>,
				TVarObjVal<Val>
				>
		{
			return VarObjVal<Val>(
				dict.value().at(MakeVarObj(std::forward<Key>(key)))
				);
		}

	template<
		typename Dict,
		typename std::enable_if_t<kIsDictType<Dict>, int> = 0
		>
		void SetVal(Dict& dict, const char* key, const char* val) {
			dict.value().insert_or_assign(MakeVarObj(key), MakeVarObj(val));
		}
	template<
		typename Dict, typename Val,
		typename std::enable_if_t<kIsDictType<Dict> && !kIsCStr<Val>, int> = 0
		>
		void SetVal(Dict& dict, const char* key, const Val& val) {
			dict.value().insert_or_assign(MakeVarObj(key), MakeVarObj(val));
		}
	template<
		typename Dict, typename Key,
		typename std::enable_if_t<kIsDictType<Dict> && !kIsCStr<Key>, int> = 0
		>
		void SetVal(Dict& dict, const Key& key, const char* val) {
			dict.value().insert_or_assign(MakeVarObj(key), MakeVarObj(val));
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && !kIsCStr<Key> && !kIsCStr<Val>, int
			> = 0
		>
		void SetVal(Dict& dict, const Key& key, const Val& val) {
			dict.value().insert_or_assign(MakeVarObj(key), MakeVarObj(val));
		}
	template<
		typename Dict, typename Key,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Key> && !kIsCStr<Key>, int
			> = 0
		>
		void SetVal(Dict& dict, Key&& key, const char* val) {
			dict.value().insert_or_assign(
				MakeVarObj(std::forward<Key>(key)), MakeVarObj(val)
				);
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Key>
				&& !kIsCStr<Key> && !kIsCStr<Val>,
			int
			> = 0
		>
		void SetVal(Dict& dict, Key&& key, const Val& val) {
			dict.value().insert_or_assign(
				MakeVarObj(std::forward<Key>(key)), MakeVarObj(val)
				);
		}
	template<
		typename Dict, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Val> && !kIsCStr<Val>, int
			> = 0
		>
		void SetVal(Dict& dict, const char* key, Val&& val) {
			dict.value().insert_or_assign(
				MakeVarObj(key), MakeVarObj(std::forward<Val>(val))
				);
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Val>
				&& !kIsCStr<Key> && !kIsCStr<Val>,
			int
			> = 0
		>
		void SetVal(Dict& dict, const Key& key, Val&& val) {
			dict.value().insert_or_assign(
				MakeVarObj(key),
				MakeVarObj(std::forward<Val>(val))
				);
		}
	template<
		typename Dict, typename Key, typename Val,
		typename std::enable_if_t<
			kIsDictType<Dict> && kIsBinONVal<Key> && kIsBinONVal<Val>
				 && !kIsCStr<Key> && !kIsCStr<Val>,
			int
			> = 0
		>
		void SetVal(Dict& dict, Key&& key, Val&& val) {
			dict.value().insert_or_assign(
				MakeVarObj(std::forward<Key>(key)),
				MakeVarObj(std::forward<Val>(val))
				);
		}

	template<typename Dict>
		auto DelKey(Dict& dict, const char* key)
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
		auto DelKey(Dict& dict, const Key& key)
			-> std::enable_if_t<
				kIsDictType<Dict> && !kIsCStr<Key>, bool
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
				kIsDictType<Dict> && kIsBinONVal<Key> && !kIsCStr<Key>, bool
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
