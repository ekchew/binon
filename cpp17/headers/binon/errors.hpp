#ifndef BINON_ERRORS_HPP
#define BINON_ERRORS_HPP

#include <any>
#include <sstream>
#include <stdexcept>

namespace binon {

	//---- Exception Hierarchy -------------------------------------------------

	//	std::exception
	//		std::logic_error
	//			std::invalid_argument
	//				TypeErr
	//					BadElemType
	//					BadTypeConv
	//						BadAnyCast
	//							BadCtnrVal
	//						BadIterType
	//						BadObjConv
	//						NonCtnrType
	//						NonTCType
	//					NoTypeCode
	//			std::out_of_range
	//				NegUnsigned
	//				TruncErr
	//					ByteTrunc
	//					IntTrunc

	//---- General Exception Types ---------------------------------------------
	//
	//	These may be generally useful outside BinON.

	//	TypeErr: an error involving data types
	struct TypeErr: std::invalid_argument {
		using std::invalid_argument::invalid_argument;
	};

	//	BadTypeConv: a failed type conversion
	struct BadTypeConv: TypeErr {
		using TypeErr::TypeErr;
	};

	//	BadAnyCast: an alternative to std::bad_any_cast
	//
	//	Typically you would throw a BadAnyCast after catching a
	//	std::bad_any_cast. The Make() class function gives you the opportunity
	//	to provide a more informative error message.
	//
	struct BadAnyCast: BadTypeConv {
		using BadTypeConv::BadTypeConv;

		//	The Make() class function helps build a sensible message string for
		//	the exception.
		//
		//	The context string would be something of the form "doing so-and-so".
		//	(It can also be "" if you prefer not to supply one.) The error
		//	message takes the form "bad any cast CONTEXT (casting from FROM_TYPE
		//	to TO_TYPE)". Here, FROM_TYPE and TO_TYPE are std::type_info::name()
		//	strings generated from the std::any-stored type and the casting
		//	target type T you supply, respectively.
		//
		//	The second argument is the returned class type. It defaults to
		//	BadAnyCast as you might expect, but if you subclass from this, you
		//	can make it that class instead.
		template<typename T, typename Cls=BadAnyCast>
			static auto Make(const std::any& obj, const std::string& context)
				-> Cls;
	};

	//	NegUnsigned signifies you are trying to assign a negative value to an
	//	unsigned integral container. In BinON terms, the automatic type
	//	conversion from IntObj to UIntObj that can happen with helper functions
	//	like GetObjVal() may throw this. BinON is quite strict in terms of not
	//	allowing conversions among its own data structures that cause any sort
	//	of loss of information.
	struct NegUnsigned: std::out_of_range {
		using std::out_of_range::out_of_range;
	};

	//	This error signifies some sort of data loss through truncation.
	//	Typically, it indicates you are losing the most-significant bits of a
	//	value through type narrowing.
	struct TruncErr: std::out_of_range {
		using std::out_of_range::out_of_range;
	};

	//	ByteTrunc throws when you try to squeeze a large integral value into a
	//	std::byte container (see byteutil.hpp).
	struct ByteTrunc: TruncErr {
		using TruncErr::TruncErr;
	};

	//	IntTrunc may throw when casting a wide integral type into a narrower one
	//	leads to data loss. BinON currently only throws this if you call
	//	IntVal::scalar() or UIntVal::scalar() when the stored value is in byte
	//	vector form and exceeds what would fit a 64-bit representation. (Note
	//	that you can call asScalar() instead if you don't mind the truncation.)
	//
	//	BinON does NOT perform a range check when you go something like
	//	GetObjVal<int>(myIntObj). There is an implied static_cast when you are
	//	converting to a basic type like int and it assumes you know what you are
	//	doing.
	struct IntTrunc: TruncErr {
		using TruncErr::TruncErr;
	};

	//---- BinON-Specific Exception Types --------------------------------------

	//	BadElemType: illegal container element type
	//
	//	This exception typically occurs when you are trying to encode a simple
	//	container type (SList, SKDict, or SDict) and fixed element, key, or
	//	value type does not match that of a particular item in the container.
	//	For example, if you had an SList with element code kStrObj and you stuck
	//	an IntObj in it. (Note: if you use high-level helper functions like
	//	MakeSList(), SetCtnrVal(), etc., such problems should be caught earlier
	//	and most likely lead to a BadObjConv exception instead when it becomes
	//	clear that an IntObj cannot convert to a StrObj.)
	struct BadElemType: TypeErr {
		using TypeErr::TypeErr;
	};

	//	BadCtnrVal: expected TValue instance missing from container object
	//
	//	Since container types like ListObj, DictObj, etc. store their value
	//	internally as a std::any, it is possible they may have been constructed
	//	with something other than they expected data type (e.g. ListObj::TValue,
	//	or std::vector<BinONObj,BINON_ALLOCATOR<BinONObj>> to be precise). This
	//	will become clear when you call value() on the container object and it
	//	throws this exception.
	struct BadCtnrVal: BadAnyCast {
		using BadAnyCast::BadAnyCast;
	};

	//	If you use a type with Iterable or ConstIterable that doesn't map onto
	//	the expected object type for a container element/key/value, you may see
	//	this exception. For example, if you had an SList with element code
	//	kStrObjCode and you tried to iterate it with an Iterable<int>, this
	//	would be trouble.
	struct BadIterType: BadTypeConv {
		using BadTypeConv::BadTypeConv;
	};

	//	BadObjConv: conversion failure between specific BinON object types
	//
	//	This may be thrown by BinONObj::AsObj(),BinONObj::asTypeCodeObj(), and
	//	other helper functions like GetObj() that call these internally.
	struct BadObjConv: BadTypeConv {
		using BadTypeConv::BadTypeConv;
	};

	//	NonCtnrType: a container type is needed
	//
	//	This exception is thrown if you try to pass a non-container type to
	//	either the Iteratable or ConstIterable class.
	struct NonCtnrType: BadTypeConv {
		using BadTypeConv::BadTypeConv;
	};

	//	NonTCType: type is unknown to the TypeConv struct
	//
	//	TypeConv is a special data structure used to map common types like int
	//	or std::string onto BinON object types like IntObj or StrObj. NonTCType
	//	comes up when you give a type it cannot handle.
	struct NonTCType: BadTypeConv {
		using BadTypeConv::BadTypeConv;
	};

	//	NoTypeCode: no type code specified for simple container
	//
	//	This would typically come up if you allowed an SList, SKDict, or SDict
	//	to default-construct and then never assigned the required
	//	element/key/value type code(s) for it before trying to encode the
	//	container.
	struct NoTypeCode: TypeErr {
		using TypeErr::TypeErr;
	};

	//==== Template Implementation =============================================

	//---- BadAnyCast ----------------------------------------------------------

	template<typename T, typename Cls>
		auto BadAnyCast::Make(
			const std::any& obj, const std::string& context
		) -> Cls
	{
		std::ostringstream oss;
		oss << "bad any cast";
		if(context.length() > 0) {
			oss << ' ' << context;
		}
		oss << " (casting from "
			<< obj.type().name() << " to "
			<< typeid(T).name() << ')';
		return Cls{oss.str()};
	}
}

#endif
