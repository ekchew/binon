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
	template<typename T, typename Enable=void>
		struct TypeInfo {
			static_assert(true, "BinON could not determine object type");

			//	Specializations provide:
			//		using Wrapper = ...;
			//		static auto TypeName() -> std::string;
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

	//==== Template Implementation ============================================

	//---- TypeInfo specializations -------------------------------------------

	template<typename T>
		struct TypeInfo<std::reference_wrapper<T>> {
			using Wrapper = typename TypeInfo<T>::Wrapper;
			static auto TypeName() -> std::string
				{ return TypeInfo<T>::TypeName(); }
		};
	template<typename T>
		struct TypeInfo<T, std::enable_if_t<kIsWrapper<T>>>
		{
			using Wrapper = T;
			static auto TypeName() -> std::string { return T{}.clsName(); }
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
		};
	template<>
		struct TypeInfo<bool> {
			using Wrapper = BoolObj;
			static auto TypeName() -> std::string { return "bool"; }
		};
	template<>
		struct TypeInfo<types::TFloat32> {
			using Wrapper = Float32Obj;
			static auto TypeName() -> std::string { return "TFloat32"; }
		};
	template<>
		struct TypeInfo<types::TFloat64> {
			using Wrapper = FloatObj;
			static auto TypeName() -> std::string { return "TFloat64"; }
		};
	template<>
		struct TypeInfo<std::string> {
			using Wrapper = StrObj;
			static auto TypeName() -> std::string { return "string"; }
		};
	template<>
		struct TypeInfo<std::string_view> {
			using Wrapper = StrObj;
			static auto TypeName() -> std::string { return "string"; }
		};
	template<>
		struct TypeInfo<StrObj::TValue> {
			using Wrapper = StrObj;
			static auto TypeName() -> std::string { return "string"; }
		};
	template<>
		struct TypeInfo<TBuffer> {
			using Wrapper = BufferObj;
			static auto TypeName() -> std::string { return "TBuffer"; }
		};

	//---- Utility Functions ---------------------------------------------------

	template<typename T>
		auto MakeSharedObj(T&& v) -> std::shared_ptr<TWrapper<T>> {
			return std::make_shared<TWrapper<T>>(std::forward<T>(v));
		}
}

#endif
