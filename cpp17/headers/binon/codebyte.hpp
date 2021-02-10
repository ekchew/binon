#ifndef BINON_CODEBYTE_HPP
#define BINON_CODEBYTE_HPP

#include "byteutil.hpp"

#include <stdexcept>
#include <istream>
#include <ostream>

namespace binon {

	struct CodeByte
		//	Wraps a std::byte containing a BinON code byte value.
	{
		template<typename I> static constexpr auto FromInt(I i) noexcept
			{ return CodeByte(ToByte(i)); }
		static auto Read(TIStream& stream, bool requireIO=true) -> CodeByte;

		constexpr CodeByte(std::byte value=0x00_byte) noexcept:
			mValue{value} {}
		constexpr operator std::byte() const noexcept {return mValue;}
		constexpr auto operator = (std::byte value) noexcept -> CodeByte&
			{ return mValue = value, *this; }
		template<typename I> constexpr auto toInt() const noexcept -> I
			{ return std::to_integer<I>(mValue); }
		constexpr auto baseType() const noexcept -> unsigned int
			{ return std::to_integer<unsigned int>(mValue >> 4); }
		constexpr auto subtype() const noexcept -> unsigned int
			{ return std::to_integer<unsigned int>(mValue & 0x0f_byte); }
		constexpr auto typeCode() const noexcept -> CodeByte
			//	A type code uniquely identifies the class of BinON object. It
			//	is identical to a regular CodeByte except it does not allow
			//	the default subtype of 0. typeCode() will raise this to the
			//	base subtype of 1.
			//
			//	Suppose you have just decoded a FloatObj with code byte 0x30.
			//	If you want to check that you are dealing with a FloatObj, you
			//	an go myObj.typeCode() == kFloatObjCode. kFloatObjCode is
			//	0x31, so the typeCode() call makes sure you are comparing 0x31
			//	to 0x31.
			{
				auto i = toInt<int>();
				if((i & 0x0F) == 0) {
					return FromInt(i + 1);
				}
				return *this;
			}
		void write(TOStream& stream, bool requireIO=true) const;
		void printRepr(std::ostream& stream) const;

	 private:
		std::byte mValue;
	};

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

	//	BaseType and Subtype are CodeByte accessor classes that let you work
	//	directly with the two fields of a code byte as unsigned int values.
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
