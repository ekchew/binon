#ifndef BINON_INTOBJ_HPP
#define BINON_INTOBJ_HPP

#include "binonobj.hpp"
#include "hystr.hpp"
#include <type_traits>
#include <variant>

namespace binon {

	//	IntBase is the parent of IntVal and UIntVal. The latter are used to
	//	store integers encoded/decoded by BinON. The data structure is a
	//	variant of an integral type and a byte vector. For IntVal (used by
	//	IntObj), the integral type is std::int64_t. For UIntVal (used by
	//	UIntObj), it is std::uint64_t.
	//
	//	The byte vector (std::vector<std::byte>) is typically used when you
	//	have integers too big to fit into a 64-bit container. Such values get
	//	stored in big-endian form in the vector.
	//
	//	This project does not provide any explicit support for 3rd party big
	//	number libraries since it was a design goal not to require external
	//	dependencies outside the standard library. But 2 methods are included
	//	to encode/decode the integer data in hexadecimal string form (using
	//	the FromHex() and asHex() methods, respectively). The GMP library, for
	//	example, supports assigning hexadecimal strings to its objects.
	template<typename Child, typename Fixed>
		struct IntBase:
			protected std::variant<Fixed, std::vector<std::byte>>
		{
			using TFixed = Fixed;
			using TVariable = std::vector<std::byte>;

			using std::variant<TFixed,TVariable>::variant;
			using std::variant<TFixed,TVariable>::operator=;

			auto isFixed() const noexcept -> bool;

			//	These methods will throw std::bad_variant_access if you choose
			//	the wrong one. Call isFixed() to check if you are unsure.
			//
			//	The asFixed() method (defined in subclasses) will narrow a
			//	variable value if necessary rather than throw. The TFixed
			//	conversion operator calls asFixed().
			auto fixed() -> TFixed&;
			auto fixed() const -> TFixed;
			auto variable() -> TVariable&;
			auto variable() const -> const TVariable&;
			operator TFixed() const noexcept;

			//	The as() method is equivalent to static_cast<T>(asFixed()).
			//	It shouldn't throw in most cases unless T's constructor does.
			//	If T is an integral type, you can call fits<T>() to check if
			//	it has enough bits to accommodate the whole value.
			template<typename T>
				auto as() const -> T;
		};
	struct IntVal: IntBase<IntVal, std::int64_t> {

		//	Reads an IntVal from a string containing hexadecimal digits.
		//	Negative numbers are indicated by the most-significant bit of the
		//	1st hex digit encountered. If you want to encode 255 rather than
		//	-1, for example, you should write IntVal::FromHex("0ff") rather
		//	than IntVal::FromHex("ff"). (Note that if you begin the string
		//	with "0x", the '0' before the 'x' is ignored from the standpoint
		//	of setting the sign. See the description under UIntVal for more
		//	general info on this class method.
		static auto FromHex(const HyStr& hex) -> IntVal;

		//	asHex() is essentially the opposite of FromHex(), generating a
		//	hexadecimal representation of the integer for you and returning it
		//	in a std::string. It takes 2 optional arguments:
		//		zerox: prepend a "0x" before the hexadecimal data?
		//			This defaults to true.
		//		wordSize: the number of bytes in a "word"
		//			This parameter helps determine how many '0' or 'f' padding
		//			digits should be inserted before the significant bits of
		//			the integer begin. The hexadecimal representation should be
		//			a multiple of wordSize bytes long.
		//
		//			Let's say the integer was 0x12345 and the wordSize was 8.
		//			Then asHex should return "0x00012345". This is particularly
		//			important when you are dealing with negative numbers, since
		//			it will determine how many 'f' digits to insert for the
		//			integer size you are considering to read negative.
		//
		//			The default 1 means asHex will insert at most 1 '0' or 'f'
		//			to pad the number out to the nearest byte boundary.
		auto asHex(
			bool zerox = true, std::size_t wordSize = 1
			) const -> std::string;

		template<typename, typename> friend struct IntBase;
		using IntBase<IntVal,TFixed>::IntBase;

		auto asFixed() const noexcept -> TFixed;

		//	Ahead of calling as() on an integral type, you can call fits<I>()
		//	to check if the currently stored value can really fit inside your
		//	type. For example, if the number is a million and you offer a
		//	std::uint16_t, fits will return false.
		//	Along somewhat similar lines is the byteCount() method, which
		//	returns how many bytes would be needed to represent the current
		//	integer. This can take a bit longer to calculate than fits().
		template<typename Int>
			auto fits() const noexcept -> bool;
		auto byteCount() const noexcept -> std::size_t;
	};
	struct UIntVal: IntBase<UIntVal, std::uint64_t> {

		//	Reads a UIntVal from a string containing hexadecimal digits.
		//	Any non-hex characters are ignored (so if your string begins
		//	with "0x", the 'x' should be skipped over as appropriate).
		//	If there are no hex digits in the string, FromHex() will return
		//	a UIntVal containing 0. Note that this method is smart enough to
		//	use a TFixed representation if the number fits.
		static auto FromHex(const HyStr& hex) -> UIntVal;
		auto asHex(
			bool zerox = true, std::size_t wordSize = 1
			) const -> std::string;

		template<typename, typename> friend struct IntBase;
		using IntBase<UIntVal,TFixed>::IntBase;
		auto asFixed() const noexcept -> TFixed;

		template<typename UInt>
			auto fits() const noexcept -> bool;
		auto byteCount() const noexcept -> std::size_t;
	};

	struct TIntObj:
		TStdAcc<TIntObj>,
		TStdEq<TIntObj>,
		TStdHash<TIntObj>,
		TStdHasDefVal<TIntObj>,
		TStdPrintArgs<TIntObj>,
		TStdCodec<TIntObj>
	{
		using TValue = std::int64_t;
		static constexpr auto kTypeCode = kIntObjCode;
		static constexpr auto kClsName = std::string_view{"TIntObj"};
		TValue mValue;
		constexpr TIntObj(TValue v = 0) noexcept: mValue{v} {}
		void encodeData(TOStream& stream, bool requireIO = true) const;
		void decodeData(TIStream& stream, bool requireIO = true);
	};
	struct TUIntObj:
		TStdAcc<TUIntObj>,
		TStdEq<TUIntObj>,
		TStdHash<TUIntObj>,
		TStdHasDefVal<TUIntObj>,
		TStdPrintArgs<TUIntObj>,
		TStdCodec<TUIntObj>
	{
		using TValue = std::uint64_t;
		static constexpr auto kTypeCode = kUIntCode;
		static constexpr auto kClsName = std::string_view{"TUIntObj"};
		TValue mValue;
		constexpr TUIntObj(TValue v = 0) noexcept: mValue{v} {}
		void encodeData(TOStream& stream, bool requireIO = true) const;
		void decodeData(TIStream& stream, bool requireIO = true);
	};

	struct IntRangeError: std::range_error {
		IntRangeError();
	};

	struct IntObj: BinONObj, Access_mValue<IntObj,std::int64_t> {
		static void EncodeData(TValue v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		IntObj(TValue v=0) noexcept: mValue{v} {}
		explicit operator bool() const noexcept override
			{ return mValue != 0; }
		auto typeCode() const noexcept -> CodeByte final {return kIntObjCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kIntObjCode &&
					mValue == static_cast<const IntObj&>(other).mValue;
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<IntObj>(mValue); }
		auto clsName() const noexcept -> std::string override
			{ return "IntObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ stream << mValue; }
	};

	struct UIntObj: BinONObj, Access_mValue<UIntObj,std::uint64_t> {
		static void EncodeData(
			TValue v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		UIntObj(TValue v=0) noexcept: mValue{v} {}
		explicit operator bool() const noexcept override
			{ return mValue != 0; }
		auto typeCode() const noexcept -> CodeByte final {return kUIntCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kUIntCode &&
					mValue == static_cast<const UIntObj&>(other).mValue;
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<UIntObj>(mValue); }
		auto clsName() const noexcept -> std::string override
			{ return "UIntObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ stream << mValue; }
	};

	namespace types {
		using UInt = UIntObj;
	}

	//==== Template Implementation =============================================

	//---- IntBase -------------------------------------------------------------

	template<typename Child, typename Fixed>
		auto IntBase<Child,Fixed>::isFixed() const noexcept -> bool {
			return std::holds_alternative<TFixed>(*this);
		}
	template<typename Child, typename Fixed>
		auto IntBase<Child,Fixed>::fixed() -> TFixed& {
			return std::get<TFixed>(*this);
		}
	template<typename Child, typename Fixed>
		auto IntBase<Child,Fixed>::fixed() const -> TFixed {
			return std::get<TFixed>(*this);
		}
	template<typename Child, typename Fixed>
		auto IntBase<Child,Fixed>::variable() -> TVariable& {
			return std::get<TVariable>(*this);
		}
	template<typename Child, typename Fixed>
		auto IntBase<Child,Fixed>::variable() const -> const TVariable& {
			return std::get<TVariable>(*this);
		}
	template<typename Child, typename Fixed>
		IntBase<Child,Fixed>::operator TFixed() const noexcept {
			return static_cast<const Child*>(this)->asFixed();
		}
	template<typename Child, typename Fixed> template<typename T>
		auto IntBase<Child,Fixed>::as() const -> T {
			return static_cast<T>(static_cast<const Child*>(this)->asFixed());
		}

	//---- IntVal --------------------------------------------------------------

	template<typename Int>
		auto IntVal::fits() const noexcept -> bool {
			using std::get_if;
			if(isFixed()) {
				if(sizeof(Int) >= sizeof(TFixed)) {
					return true;
				}
				auto limit = 1ULL << (sizeof(Int) * 8 - 1);
				auto i = *get_if<TFixed>(this);
				return -limit <= i && i < limit;
			}
			return get_if<TVariable>(this)->size() <= sizeof(Int);
		}

	//---- UIntVal -------------------------------------------------------------

	template<typename UInt>
		auto UIntVal::fits() const noexcept -> bool {
			using std::get_if;
			if(isFixed()) {
				if(sizeof(UInt) >= sizeof(TFixed)) {
					return true;
				}
				auto limit = 1ULL << (sizeof(UInt) * 8);
				auto i = *get_if<TFixed>(this);
				return i < limit;
			}
			return get_if<TVariable>(this)->size() <= sizeof(UInt);
		}
}

namespace std {
	template<> struct hash<binon::IntObj> {
		constexpr auto operator () (const binon::IntObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
	template<> struct hash<binon::UIntObj> {
		 constexpr auto operator () (const binon::UIntObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
