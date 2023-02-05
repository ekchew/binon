#ifndef BINON_CODEBYTE_HPP
#define BINON_CODEBYTE_HPP

#include "byteutil.hpp"

#include <stdexcept>
#include <istream>
#include <ostream>

namespace binon {

	//---- CodeByte Class -----------------------------------------------------
	//
	//	Wraps a std::byte containing a BinON code byte value.

	class CodeByte {
		std::byte _value;
	
	public:

		//---- Factory Functions ----------------------------------------------

		//	FromInt class method
		//
		//	Makes a CodeByte out of an integer in the range [0,255]
		//	(or [-128,255] if signed).
		//
		static constexpr auto FromInt(std::integral auto i) noexcept
			{ return CodeByte(ToByte(i)); }
		
		//	Read class method
		//
		//	Reads a CodeByte from an input stream.
		//
		//	Args:
		//		stream: an input stream
		//			(This should be a std::istream unless you modify TIStream's
		//			definition in ioutil.hpp.)
		//		requireIO: throw std::failure if read fails? (defaults to true)
		//
		static auto Read(TIStream& stream, bool requireIO=true) -> CodeByte;

		//---- Main Constructor -----------------------------------------------
		//
		//	This takes a std::byte which defaults to 0x00_byte.
		//	(See byteutil.hpp regarding _byte literals.)
		//
		//	Other constructors and assignment operators like copy/move are
		//	implicitly defined.

		constexpr CodeByte(std::byte value=0x00_byte) noexcept:
			_value{value} {}
		
		//---- Accessors ------------------------------------------------------

		//	A CodeByte implicitly converts to std::byte in a read-only
		//	capacity.
		constexpr operator std::byte() const noexcept {return _value;}

		//	asInt method
		//
		//	Returns the byte value as an integer whose type you can specify in
		//	a template arg. (It defaults to unsigned int.)
		//
		template<typename Int=unsigned int> constexpr
			auto asInt() const noexcept -> Int {
				return std::to_integer<I>(_value);
			}
		
		//	baseType, subtype methods
		//
		//	These extract the 2 values making up a CodeByte in unsigned int
		//	form. (See also the BaseType and Subtype accessor classes defined
		//	further down.)
		//
		constexpr auto baseType() const noexcept -> unsigned int
			{ return std::to_integer<unsigned int>(_value >> 4); }
		constexpr auto subtype() const noexcept -> unsigned int
			{ return std::to_integer<unsigned int>(_value & 0x0f_byte); }

		//	typeCode method
		//
		//	A type code uniquely identifies the class of a BinON object. It is
		//	identical to a regular CodeByte except it does not allow the
		//	default subtype of 0. typeCode() will raise this to the base
		//	subtype of 1.
		//
		//	Suppose you have just decoded a FloatObj with code byte 0x30. If
		//	you want to check that you are dealing with a FloatObj, you an go
		//	myObj.typeCode() == kFloatObjCode. kFloatObjCode is 0x31, so the
		//	typeCode() call makes sure you are comparing 0x31 to 0x31.
		//
		constexpr auto typeCode() const noexcept -> CodeByte {
				auto i = asInt<int>();
				if((i & 0x0F) == 0) {
					return FromInt(i + 1);
				}
				return *this;
			}

		//---- Output Methods--------------------------------------------------

		//	write method
		//
		//	Writes a CodeByte to an output stream. (See also Read().)
		//
		//	Args:
		//		stream: the output stream
		//			(This should be a std::ostream unless you modify TOStream's
		//			definition in ioutil.hpp.)
		//		requireIO: throw std::failure if read fails? (defaults to true)
		//
		void write(TOStream& stream, bool requireIO=true) const;

		//	printRepr method
		//
		//	This prints the current type code as the name of one of the
		//	type code constant defined further down (e.g. "kIntObjCode").
		//	(See also the overloaded << operator.)
		//
		//	Args:
		//		stream: a std::ostream
		//			(Unlike with write(), you cannot change this type since
		//			it is not for writing BinON binary data.)
		//
		void printRepr(std::ostream& stream) const;

	};
	
	//	CodeByte's << operator is overloaded to call printRepr() when printing
	//	to a std::ostream.
	auto operator << (
		std::ostream& stream, const CodeByte& cb) -> std::ostream&;

	//-------------------------------------------------------------------------

	namespace details {

		//	A CRTP (curiously recurring template pattern) parent class for the
		//	BaseType and Subtype classes that follow.
		template<typename Subcls> struct CodeBaseField {
			constexpr CodeBaseField(CodeByte& codeByte) noexcept:
				mCodeByte{codeByte} {}

			//	CRTP lets you add extra operators like this without a lot of
			//	duplication. May add more as the need arises?
			constexpr auto operator += (unsigned int i) noexcept -> Subcls&
				{ auto& sc = subcls(); return sc = sc + i, sc; }
			constexpr auto operator -= (unsigned int i) noexcept -> Subcls&
				{ auto& sc = subcls(); return sc = sc - i, sc; }

		 protected:
			CodeByte& mCodeByte;

			constexpr auto subcls() noexcept -> Subcls&
				{ return *static_cast<Subcls*>(this); }
		};

	}

	//---- BaseType and Subtype Classes ---------------------------------------
	//
	//	These are essentially referency proxy classes that let you access the
	//	base and subtype fields within a CodeByte as if they were independent
	//	variables.

	class BaseType: public details::CodeBaseField<BaseType> {
	public:
		using details::CodeBaseField<BaseType>::CodeBaseField;
		constexpr operator unsigned int() const noexcept
			{ return mCodeByte.toInt<unsigned int>() >> 4; }
		constexpr auto operator = (unsigned int value) noexcept -> BaseType&
			{ return mCodeByte = ToByte(value << 4), *this; }
	};

	class Subtype: public details::CodeBaseField<Subtype> {
	public:

		//	Constants defining default and base subtypes.
		static constexpr unsigned int
			kDefault{0},
			kBase{1};

		using details::CodeBaseField<Subtype>::CodeBaseField;
		constexpr operator unsigned int() const noexcept
			{ return mCodeByte.toInt<unsigned int>() & 0x0fu; }
		constexpr auto operator = (unsigned int value) noexcept -> Subtype& {
				return mCodeByte =
					mCodeByte     & 0xF0_byte |
					ToByte(value) & 0x0F_byte, *this;
			}
	};

	//---- Type Codes ---------------------------------------------------------

	//	Type codes for every BinONObj type.
	constexpr CodeByte
		kNullObjCode{0x01_byte},
		kBoolObjCode{0x11_byte},
		kTrueObjCode{0x12_byte},
		kIntObjCode{0x21_byte},
		kUIntCode{0x22_byte},
		kFloatObjCode{0x31_byte},
		kFloat32Code{0x32_byte},
		kBufferObjCode{0x41_byte},
		kStrObjCode{0x51_byte},
		kListObjCode{0x81_byte},
		kSListCode{0x82_byte},
		kDictObjCode{0x91_byte},
		kSKDictCode{0x92_byte},
		kSDictCode{0x93_byte};

	//	Place-holder code for when the default constructor is invoked in
	//	simple list or dict types.
	constexpr CodeByte
		kNoObjCode{0xff_byte};

	//-------------------------------------------------------------------------

	class BadCodeByte: public std::range_error {
	 public:
		BadCodeByte(CodeByte cb): std::range_error{WhatStr(cb)} {}

	 private:
		static auto WhatStr(CodeByte cb) -> std::string;
	};
}

namespace std {
	template<> struct hash<binon::CodeByte> {
		inline auto operator () (const binon::CodeByte& v) const noexcept
			-> std::size_t {
				return std::hash<std::uint8_t>{}(v.toInt<std::uint8_t>());
			}
	};
}

#endif
