#ifndef BINON_TYPECONV_HPP
#define BINON_TYPECONV_HPP

#include "binonobj.hpp"

#include <sstream>

namespace binon {

	//	kIsCStr tells you whether a given type is a C string. It's used by some
	//	of the template specializations used helper functions.
	template<typename T>
		constexpr bool kIsCStr = std::is_same_v<std::decay_t<T>,const char*>;
	BINON_IF_CONCEPTS(
		template<typename T> concept CStrType = kIsCStr<T>;
		template<typename T> concept NonCStr = !kIsCStr<T>;
	)


	//	Some nifty code off the Internet that determines if a given type is
	//	among the possible members of a std::variant.
	template<typename T, typename Variant> struct IsVariantMember;
	template<typename T, typename... EveryT>
		struct IsVariantMember<T, std::variant<EveryT...>>:
			std::disjunction<std::is_same<T, EveryT>...>
		{
		};
	template<typename T, typename Variant>
		constexpr bool kIsVariantMember = IsVariantMember<T,Variant>::value;
	#if BINON_CONCEPTS
		template<typename Variant, typename T>
			concept VariantMember = kIsVariantMember<T,Variant>;
	#endif

	//	This uses the above to tell you if a give type T is a specific BinON
	//	object type like IntObj, StrObj, etc.
	template<typename T>
		constexpr bool kIsObj
			= kIsVariantMember<std::decay_t<T>,BinONVariant>;
	BINON_IF_CONCEPTS(
		template<typename T> concept ObjType = kIsObj<T>;
	)

	/*
	The TypeConv struct maps common types onto BinON object types for easy
	conversion between the two. You usually don't access it directly, since
	higher-level functions like MakeObj(), BinONObjVal(), and the various
	list and dict helper functions do so for you.

	TypeConv supports the following mappings:

		General Type          BinON Obj   Notes
		____________          _________   _____

		bool                  BoolObj     native value type
		BoolObj               BoolObj
		int8_t                IntObj
		int16_t               IntObj
		int32_t               IntObj
		int64_t               IntObj
		IntVal                IntObj      native value type
		IntObj                IntObj
		uint8_t               UIntObj
		uint16_t              UIntObj
		uint32_t              UIntObj
		uint64_t              UIntObj
		UIntVal               IntObj      native value type
		UIntObj               UIntObj
		TFloat32              Float32Obj  native value type, see floattypes.hpp
		Float32Obj            Float32Obj
		TFloat64              FloatObj    native value type, see floattypes.hpp
		FloatObj              FloatObj
		BufferVal             BufferObj   native value type
		BufferObj             BufferObj
		std::string           StrObj
		std::string_view      StrObj
		HyStr                 StrObj      native value type, see hystr.hpp
		const char*           StrObj      BinONObjVal() returns std::string_view
		TStringObj            StrObj
		std::vector<BinONObj> ListObj     native value type
		ListObj               ListObj
		SList                 SList
		std::unordered_map<BinONObj,BinONObj>
		                      DictObj     native value type
		DictObj               DictObj
		SKDict                SKDict
		SDict                 SDict

	First of all, you can see that all BinON object types map onto themselves as
	you might expect. In fact, for list and dictionary subtypes like SList, the
	only way to specify them in the functions found here is to supply the object
	type directly. That's because they all share the same value type as the base
	type. For example, ListObj::TValue and SList::TValue are the same type:
	std::vector<BinONObj>. (That means technically, you could mix object types
	even in a SList, but when you go to encode it, you will get a
	binon::TypeErr exception. So don't!)

	Where you see "native value type" in the notes, this is the type the
	enclosing object uses to store its data. For example, BoolObj::TValue is
	bool, and calling value() on a BoolObj will return a bool. Native value
	types get special treatment in that they can be moved (not just copied)
	into and out of objects since there is no need to convert the data type.
	Also, getter functions (e.g. BinONObjVal()) can return the value by reference
	rather than value. Provided the BinONObj in question is not a constant, you
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
			static auto GetVal(BinONObj) -> TVal;

			TObj is the data type of the BinON object that would wrap the type
			you supply. So for example, TypeConv<bool>::TObj would be BoolObj.

			TVal is typically your supplied type T with a few exceptions:
				- If T is a BinON object type, TVal will be its internal value
				  type. For example, TypeConv<BoolObj>::TVal is
				  BoolObj::TValue which, in turn, is simply bool.
				- If T is a std::reference_wrapper<U>, TVal will be U.
				- There is a special case for when T is const char*.
				  Setter functions like MakeObj() will treat a const char*
				  as a C string and build StrObj variants out of them.
				  But a StrObj cannot return a C string, so getter functions
				  like BinONObjVal() return the next best thing: a string view.
				  So for T = const char*, TVal will be std::string_view.

			ValTypeName() simply returns a string naming the TVal type which
			could be useful in debugging? For example, if you make T a 32-bit
			integer, the returned type name would be HyStr{"int32_t"}. Note
			that you can also get the name of a BinON class from its kClsName
			field (e.g. BoolObj::kClsName is HyStr{"BoolObj"}).

			The GetVal() class method takes a BinONObj and attempts to extract
			a value of type TVal from it. Typically, you would call BinONObjVal()
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
	TValObj<T> gives you the BinON object type corresponding to your type T.
	For example, TValObj<int> is IntObj. It is equivalent to TypeConv<T>::TObj
	except it decays T first to remove any const/reference qualifiers and such.
	*/
	template<typename T>
		using TValObj = typename TypeConv<std::decay_t<T>>::TObj;

	/*
	kIsBinONVal<T> evaluates true if your type T is the native value type of a
	BinON object. This would be the type that is denoted TValue in the class.
	For example, HyStr is a BinON value because it is StrObj::TValue.

	BinON value types are useful because if you specify one in the helper
	functions defined elsewhere, you can access the value directly and even
	modify it in-place, since it's not one of these data types that need to be
	converted first. BinON value types can also be moved--not just
	copied--into/out of place.
	*/
	template<typename T>
		constexpr bool kIsBinONVal =
			std::is_same_v<
				std::decay_t<T>,
				typename TypeConv<std::decay_t<T>>::TObj::TValue
				>;
	BINON_IF_CONCEPTS(
		template<typename T> concept BinONVal = kIsBinONVal<T>;
		template<typename T> concept NonBinONVal = !kIsBinONVal<T>;
	)


	//==== Template Implementation =============================================

	//---- TypeConv specializations --------------------------------------------

	template<typename T>
		struct TypeConv<T, std::reference_wrapper<T>> {
			using TObj = typename TypeConv<T>::TObj;
			using TVal = typename TypeConv<T>::TVal;
			static auto ValTypeName() -> HyStr {
					return TypeConv<T>::ValTypeName();
				}
			static auto GetVal(BinONObj& obj) -> TVal& {
					return TypeConv<T>::GetVal(obj);
				}
			static auto GetVal(const BinONObj& obj) -> TVal {
					return TypeConv<T>::GetVal(obj);
				}
			static auto GetVal(BinONObj&& obj) -> TVal {
					return TypeConv<T>::GetVal(std::forward<BinONObj>(obj));
				}
		};
 #if BINON_CONCEPTS
	template<ObjType T>
		struct TypeConv<T>
 #else
	template<typename T>
		struct TypeConv<T, std::enable_if_t<kIsObj<T>>>
 #endif
		{
			using TObj = T;
			using TVal = typename TObj::TValue;
			static auto ValTypeName() -> HyStr { return TObj::kClsName; }
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(BinONObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<BinONObj>(obj)).value();
				}
		};
 #if BINON_CONCEPTS
	template<std::signed_integral T>
		struct TypeConv<T>
 #else
	template<typename T>
		struct TypeConv<
			T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>
			>
 #endif
		{
			using TObj = IntObj;
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
			static auto GetVal(const BinONObj& obj) -> TVal {
					return static_cast<TVal>(
						obj.asObj<TObj,UIntObj>().value().scalar()
					);
				}
			static auto GetVal(BinONObj&& obj) -> TVal {
					return static_cast<TVal>(
						std::move(obj).asObj<TObj,UIntObj>().value().scalar()
					);
				}
		};
 #if BINON_CONCEPTS
	template<std::unsigned_integral T>
		struct TypeConv<T>
 #else
	template<typename T>
		struct TypeConv<
			T, std::enable_if_t<std::is_unsigned_v<T>>
			>
 #endif
		{
			using TObj = UIntObj;
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
			static auto GetVal(const BinONObj& obj) -> TVal {
					return static_cast<TVal>(
						obj.asObj<TObj,IntObj>().value().scalar()
					);
				}
			static auto GetVal(BinONObj&& obj) -> TVal {
					return static_cast<TVal>(
						std::move(obj).asObj<TObj,IntObj>().value().scalar()
					);
				}
		};
	template<>
		struct TypeConv<IntVal> {
			using TObj = IntObj;
			using TVal = IntVal;
			static auto ValTypeName() -> HyStr { return "IntVal"; }
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<UIntVal> {
			using TObj = UIntObj;
			using TVal = UIntVal;
			static auto ValTypeName() -> HyStr { return "UIntVal"; }
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<bool> {
			using TObj = BoolObj;
			using TVal = bool;
			static auto ValTypeName() -> HyStr { return "bool"; }
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<types::TFloat64> {
			using TObj = FloatObj;
			using TVal = types::TFloat64;
			static auto ValTypeName() -> HyStr { return "TFloat64"; }
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<types::TFloat32> {
			using TObj = Float32Obj;
			using TVal = types::TFloat32;
			static auto ValTypeName() -> HyStr { return "TFloat32"; }
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<HyStr> {
			using TObj = StrObj;
			using TVal = HyStr;
			static auto ValTypeName() -> HyStr { return "HyStr"; }
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(BinONObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<BinONObj>(obj)).value();
				}
		};
	template<>
		struct TypeConv<std::string> {
			using TObj = StrObj;
			using TVal = std::string;
			static auto ValTypeName() -> HyStr { return "string"; }
			static auto GetVal(const BinONObj& obj) -> TVal {
					return std::get<TObj>(obj).value().asStr();
				}
		};
	template<>
		struct TypeConv<std::string_view> {
			using TObj = StrObj;
			using TVal = std::string_view;
			static auto ValTypeName() -> HyStr { return "string_view"; }
			static auto GetVal(const BinONObj& obj) -> TVal {
					return std::get<TObj>(obj).value().asView();
				}
		};
 #if BINON_CONCEPTS
	template<CStrType T>
		struct TypeConv<T>
 #else
	template<typename T>
		struct TypeConv<T, std::enable_if_t<kIsCStr<T>>>
 #endif
		{
			using TObj = StrObj;
			using TVal = std::string_view;
			static auto ValTypeName() -> HyStr { return "const char*"; }
			static auto GetVal(const BinONObj& obj) -> TVal {
					return std::get<TObj>(obj).value().asView();
				}
		};
	template<>
		struct TypeConv<BufferVal> {
			using TObj = BufferObj;
			using TVal = BufferVal;
			static auto ValTypeName() -> HyStr { return "BufferVal"; }
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(BinONObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<BinONObj>(obj)).value();
				}
		};
	template<>
		struct TypeConv<ListObj::TValue> {
			using TObj = ListObj;
			using TVal = ListObj::TValue;
			static auto ValTypeName() -> HyStr { return "vector<BinONObj>"; }
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(BinONObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<BinONObj>(obj)).value();
				}
		};
	template<>
		struct TypeConv<DictObj::TValue> {
			using TObj = DictObj;
			using TVal = DictObj::TValue;
			static auto ValTypeName() -> HyStr {
					return "unordered_map<BinONObj,BinONObj>";
				}
			static auto GetVal(BinONObj& obj) -> TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(const BinONObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(BinONObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<BinONObj>(obj)).value();
				}
		};
}

#endif
