#ifndef BINON_TYPEINFO_HPP
#define BINON_TYPEINFO_HPP

#include "boolobj.hpp"
#include "bufferobj.hpp"
#include "floatobj.hpp"
#include "intobj.hpp"
#include "strobj.hpp"

#include <memory>

namespace binon {

	//	This class template helps map simple data types to their BinON wrapper
	//	class counterparts. For example, TypeInfo<bool>::Wrapper (or
	//	TWrapper<bool> for short) would be BoolObj. The class also provides a
	//	TypeName() method which, in this case, would return the std::string
	//	"bool". It is used internally by SListT and the like (see below).
	//
	//	As it stands, TypeInfo makes the following mappings of basic types:
	//
	//		Type Arg         Wrapper   Type Name
	//		________         _______    _________
	//
	//		bool             BoolObj    "bool"
	//		int8_t           IntObj     "int8_t"
	//		int16_t          IntObj     "int16_t"
	//		int32_t          IntObj     "int32_t"
	//		int64_t          IntObj     "int64_t"
	//		uint8_t          UIntObj    "uint8_t"
	//		uint16_t         UIntObj    "uint16_t"
	//		uint32_t         UIntObj    "uint32_t"
	//		uint64_t         UIntObj    "uint64_t"
	//		TFloat32         Float32Obj "TFloat32"
	//		TFloat64         FloatObj   "TFloat64"
	//		TBuffer          BufferObj  "TBuffer"
	//		std::string      StrObj     "string"
	//		std::string_view StrObj     "string"
	//		HyStr            StrObj     "string"
	//
	//	Also, if you feed it a type is already a wrapper class, you will simply
	//	get the wrapper. In other words, TypeInfo<<BoolObj>::Wrapper is BoolObj
	//	and TypeInfo<BoolObj>::Name() is "BoolObj".
	//
	//	GetValue() method:
	//		This convenience method is meant to help you pull the value out of
	//		BinONObj you've decoded from somewhere. For example, you could
	//		write:
	//
	//			int i = TypeInfo<int>::GetValue(pObj);
	//
	//		Internally, this calls BinONObj::Cast(), so it may throw an
	//		exception if the object you supply is not of the correct type
	//		(IntObj in this case). (You can call pObj->typeCode() if you're
	//		not sure.)
	//
	//		If you specify a BinONObj type rather than a simple type like the
	//		int above, GetValue() should return the mValue field of the object--
	//		whatever type that might be.
	//
	//		Args:
	//			pObj (std::shared_ptr<BinONObj>): pointer to an object
	//
	//		Returns: the value contained within the object

	template<typename T, typename Enable=void>
		struct TypeInfo {
			static_assert(true, "BinON could not determine object type");

			//	Specializations provide:
			//		using Wrapper = ...;
			//		static auto TypeName() -> std::string;
			//		static auto GetValue(TSPBinONObj pObj) -> T;
		};
	template<typename T> using TWrapper = typename TypeInfo<T>::Wrapper;

	//	kIsWrapper<BoolObj> evaluates to true to indicate that you are already
	//	looking at a wrapper class while kIsWrapper<bool> evaluates false.
	template<typename T> inline constexpr
		bool kIsWrapper = std::is_base_of_v<BinONObj, T>;
	
	/**
		MakeSharedObj function

		A simple function that allocates a shared pointer to a BinON object of
		the appropriate type based on the argument you supply. For example,
		MakeSharedPtr(42) would return a std::shared_ptr<IntObj>.

		Args:
			v: a value of a type TypeInfo understands

		Returns: a shared pointer containing the value
	**/
	template<typename T>
		auto MakeSharedObj(T&& v) -> std::shared_ptr<TWrapper<T>>;

	template<typename T>
		auto SharedObjVal(TSPBinONObj pObj) -> T&&;

	//==== Template Implementation ============================================

	//---- TypeInfo specializations -------------------------------------------

	template<typename T>
		struct TypeInfo<std::reference_wrapper<T>> {
			using Wrapper = typename TypeInfo<T>::Wrapper;
			static auto TypeName() -> std::string
				{ return TypeInfo<T>::TypeName(); }
			static auto GetValue(const TSPBinONObj pObj)
				{ return TypeInfo<T>::GetValue(pObj); }
		};
	template<typename T>
		struct TypeInfo<T, std::enable_if_t<kIsWrapper<T>>>
		{
			using Wrapper = T;
			static auto TypeName() -> std::string { return T{}.clsName(); }
			static auto GetValue(const TSPBinONObj pObj)
					-> const typename T::TValue&
				{
					return BinONObj::Cast<const Wrapper>(pObj)->mValue;
				}
		};
	template<typename T>
		struct TypeInfo<
			T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>
			>
		{
			using Wrapper = IntObj;
			static auto TypeName() -> std::string {
				switch(sizeof(T)) {
						case 1: return "int8_t";
						case 2: return "int16_t";
						case 4: return "int32_t";
						case 8: return "int64_t";
						default: return "SIGNED_INTEGER";
					}
				}
			static auto GetValue(const TSPBinONObj pObj) -> T {
					return static_cast<T>(
						BinONObj::Cast<const Wrapper>(pObj)->mValue
						);
				}
		};
	template<typename T>
		struct TypeInfo<T, std::enable_if_t<std::is_unsigned_v<T>>> {
			using Wrapper = UIntObj;
			static auto TypeName() -> std::string {
				switch(sizeof(T)) {
						case 1: return "uint8_t";
						case 2: return "uint16_t";
						case 4: return "uint32_t";
						case 8: return "uint64_t";
						default: return "UNSIGNED_INTEGER";
					}
				}
			static auto GetValue(const TSPBinONObj pObj) -> T {
					return static_cast<T>(
						BinONObj::Cast<const Wrapper>(pObj)->mValue
						);
				}
		};
	template<>
		struct TypeInfo<bool> {
			using Wrapper = BoolObj;
			static auto TypeName() -> std::string { return "bool"; }
			static auto GetValue(const TSPBinONObj pObj) -> bool {
					return BinONObj::Cast<const Wrapper>(pObj)->mValue;
				}
		};
	template<>
		struct TypeInfo<types::TFloat32> {
			using Wrapper = Float32Obj;
			static auto TypeName() -> std::string { return "TFloat32"; }
			static auto GetValue(const TSPBinONObj pObj)
				-> types::TFloat32
				{
					return BinONObj::Cast<const Wrapper>(pObj)->mValue;
				}
		};
	template<>
		struct TypeInfo<types::TFloat64> {
			using Wrapper = FloatObj;
			static auto TypeName() -> std::string { return "TFloat64"; }
			static auto GetValue(const TSPBinONObj pObj)
				-> types::TFloat64
				{
					return BinONObj::Cast<const Wrapper>(pObj)->mValue;
				}
		};
	template<>
		struct TypeInfo<std::string> {
			using Wrapper = StrObj;
			static auto TypeName() -> std::string { return "string"; }
			static auto GetValue(const TSPBinONObj pObj) -> std::string {
					return BinONObj::Cast<const Wrapper>(pObj)->mValue.asStr();
				}
		};
	template<>
		struct TypeInfo<std::string_view> {
			using Wrapper = StrObj;
			static auto TypeName() -> std::string { return "string"; }
			static auto GetValue(const TSPBinONObj pObj)
				-> std::string_view
				{
					return BinONObj::Cast<const Wrapper>(pObj)->mValue.asView();
				}
		};
	template<>
		struct TypeInfo<StrObj::TValue> {
			using Wrapper = StrObj;
			static auto TypeName() -> std::string { return "string"; }
			static auto GetValue(const TSPBinONObj pObj)
				-> const StrObj::TValue&
				{
					return BinONObj::Cast<const Wrapper>(pObj)->mValue;
				}
		};
	template<>
		struct TypeInfo<TBuffer> {
			using Wrapper = BufferObj;
			static auto TypeName() -> std::string { return "TBuffer"; }
			static auto GetValue(const TSPBinONObj pObj)
				-> const TBuffer&
				{
					return BinONObj::Cast<Wrapper>(pObj)->mValue;
				}
		};

	//---- Utility Functions ---------------------------------------------------

	template<typename T>
		auto MakeSharedObj(T&& v) -> std::shared_ptr<TWrapper<T>> {
			return std::make_shared<TWrapper<T>>(std::forward<T>(v));
		}
}

#endif
