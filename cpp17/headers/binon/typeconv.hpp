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

	template<typename T, typename Enable = void>
		struct TypeConv {
			static_assert(true, "Cannot convert type to/from BinON object");
			/*
			using TObj = ...
			using TVal = ...
			static auto ValTypeName() -> HyStr;
			static auto GetVal(const TVarObj& obj) -> TVal;
			static auto GetVal(TVarObj&& obj) -> TVal;
			*/
		};

	template<typename T>
		auto MakeVarObj(const T& v) -> TVarObj;
	template<typename T>
		auto MakeVarObj(T&& v) -> TVarObj;

	template<typename T>
		using TVarObjVal = typename TypeConv<std::decay_t<T>>::TVal;
	template<typename T>
		auto VarObjVal(const TVarObj& obj) -> TVarObjVal<T>&&;
	template<typename T>
		auto VarObjVal(TVarObj&& obj) -> TVarObjVal<T>&&;

	//==== Template Implementation =============================================

	template<typename T>
		auto MakeVarObj(const T& v) -> TVarObj {
			return typename TypeConv<T>::TObj(v);
		}
	template<typename T>
		auto MakeVarObj(T&& v) -> TVarObj {
			return typename TypeConv<T>::TObj(std::forward<T>(v));
		}
	template<typename T>
		auto VarObjVal(const TVarObj& obj) -> TVarObjVal<T>&& {
			return TypeConv<std::decay<T>>::GetVal(obj);
		}
	template<typename T>
		auto VarObjVal(TVarObj&& obj) -> TVarObjVal<T>&& {
			return TypeConv<std::decay<T>>::GetVal(std::forward<TVarObj>(obj));
		}

	//---- TypeConv specializations -------------------------------------------

	template<typename T>
		struct TypeConv<T, std::reference_wrapper<T>> {
			using TObj = typename TypeConv<T>::TObj;
			using TVal = typename TypeConv<T>::TVal;
			static auto ValTypeName() -> HyStr {
					return TypeConv<T>::ValTypeName();
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
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<TUIntVal> {
			using TObj = TUIntObj;
			using TVal = TUIntVal;
			static auto ValTypeName() -> HyStr { return "TUIntVal"; }
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<bool> {
			using TObj = TBoolObj;
			using TVal = bool;
			static auto ValTypeName() -> HyStr { return "bool"; }
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<types::TFloat64> {
			using TObj = TFloatObj;
			using TVal = types::TFloat64;
			static auto ValTypeName() -> HyStr { return "TFloat64"; }
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<types::TFloat32> {
			using TObj = TFloat32Obj;
			using TVal = types::TFloat32;
			static auto ValTypeName() -> HyStr { return "TFloat32"; }
			static auto GetVal(const TVarObj& obj) -> TVal {
					return std::get<TObj>(obj).value();
				}
		};
	template<>
		struct TypeConv<HyStr> {
			using TObj = TStrObj;
			using TVal = HyStr;
			static auto ValTypeName() -> HyStr { return "HyStr"; }
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
		struct TypeConv<TBufferVal> {
			using TObj = TBufferObj;
			using TVal = TBufferVal;
			static auto ValTypeName() -> HyStr { return "TBufferVal"; }
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
			static auto GetVal(const TVarObj& obj) -> const TVal& {
					return std::get<TObj>(obj).value();
				}
			static auto GetVal(TVarObj&& obj) -> TVal {
					return std::get<TObj>(std::forward<TVarObj>(obj)).value();
				}
		};
}

#endif
