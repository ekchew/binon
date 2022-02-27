#ifndef BINON_INTOBJ_HPP
#define BINON_INTOBJ_HPP

#include "hystr.hpp"
#include "mixins.hpp"
#include <ostream>
#include <type_traits>
#include <variant>

namespace binon {
	//	This header defines the IntObj and UIntObj structures as well as the
	//	IntVal and UIntVal structures they use as their TValue types. (Note that
	//	you can use the namespace binon::types if you want to refer to UInt
	//	instead of UIntObj in keeping with the Python interface.)
	//
	//	The value types contain take 2 forms internally: scalar or vector. This
	//	is done using a std::variant between a 64-bit integer (std::int64_t or
	//	std::uint64_t for IntVal or UIntVal, respectively) and a
	//	std::basic_string<std::byte> for the vector form.
	//
	//	The vector form is needed to represent values that are too large to fit
	//	in 64 bits. When used, the byte string should follow the big-endian byte
	//	ordering convention. For IntVal, the most-significant bit of the
	//	most-significant byte is considered the sign bit. If you wanted to
	//	represent 128 rather than -128, then, you would need to insert a 0x00
	//	byte before the 0x80 byte.
	//
	//	BinON does not implement a big integer library. However, you may find
	//	the asHex() and FromHex() methods useful for converting to/from a form
	//	suitable to third party libraries?

	struct UIntObj;

	//	IntBase is an abstract CRTP base class that implements common
	//	functionality between IntVal and UIntVal. It, in turn, inherits all
	//	public functionality (including constructors) from a std::variant of the
	//	scalar and vector forms described earlier.
	template<typename Child, typename Scalar>
		struct IntBase: std::variant<Scalar, std::basic_string<std::byte>>
		{
			using TScalar = Scalar; // std::int64_t or std::uint64_t
			using TVect = std::basic_string<std::byte>;

			using std::variant<TScalar,TVect>::variant;

			//---- Scalar form accessors ---------------------------------------
			//
			//	isScalar() tells you if the value is in scalar rather than
			//	vector form, typically after normalizing it.
			//
			//	scalar() attempts to return the scalar value but may fail,
			//	throwing IntTrunc instead if its a huge number that cannot be
			//	normalized to scalar form. If you call scalar(kSkipNormalize),
			//	it will skip trying to convert vector to scalar form altogether.
			//	In this case, you might get a std::bad_variant_access exception
			//	instead if the value is in vector form.
			//
			//	scalar() with normalization can also be invoked through a
			//	TScalar conversion operator. But beware that it has the
			//	potential to throw IntTrunc!
			//
			//	asScalar() (defined in subclasses) is like scalar() except it
			//	will silently truncate the integer rather than throw an
			//	exception. It automatically calls normalize(). Helper functions
			//	like GetObjVal() call asScalar() when dealing with integer
			//	objects.
			//
			//	asNum() is simply asScalar() with a static cast to whatever type
			//	you like.
			//
			//	fits() (defined in subclasses) tells you whether an integral
			//	type you supply can accommodate the currently stored value
			//	without truncation. You might call it before asNum() on the same
			//	type if you're worried about this.

			auto isScalar(bool normalize=true) const -> bool;
			auto scalar(bool normalize=true) const -> TScalar;
			operator TScalar() const;
		 //	auto asScalar() const noexcept -> TScalar;
			template<typename N>
				auto asNum() const -> N;
		 //	template<typename Int>
		 //		auto fits() const noexcept -> bool;

			//---- Vector form accessors ---------------------------------------
			//
			//	vect() is equivalent to calling std::get() to extract the TVect
			//	byte string. It will throw std::bad_variant_access should it
			//	fail.
			//
			//	asVect() will convert the scalar form to a temporary TVect if it
			//	needs to, rather than throw std::bad_variant_access. (The only
			//	exception it might conceivably throw would be std::bad_alloc if
			//	it runs out of memory trying to allocate a new byte string
			//	buffer.)
			//
			//	accessVect() is like asVect() except it passes the vector by
			//	reference to a callback you provide rather than simply return
			//	it. The advantage of this approach is if the variant is already
			//	in vector form, it doesn't have to make a copy of the whole
			//	thing to return. The return value of accessVect() itself should
			//	be whatever you want to return from your callback.

			auto vect() -> TVect&;
			auto vect() const -> const TVect&;
			auto asVect() const -> TVect;
			template<typename ReturnType=void>
				auto accessVect(
					std::function<ReturnType(const TVect&)> callback
					) const -> ReturnType;

			//	This method does nothing if the variant is in scalar form.
			//	Otherwise, it makes sure the vector is as small as it can be. If
			//	there are any pad bytes at the beginning that do not contain any
			//	significant bits, these get stripped off. Then it checks to see
			//	if the vector has no more than 64 bits, in which case it will
			//	convert it to the scalar form. Finally, if it is still in vector
			//	form and you requested shrinkToFit, shrink_to_fit() will be
			//	called on the byte string.
			//
			//	All of this may modify the object despite the const qualifier,
			//	but the integer value itself should remain identical.
			void normalize(bool shrinkToFit=false) const;

			//---- Hexadecimal conversion --------------------------------------
			//
			//	FromHex() (defined in subclasses) parses a string containing
			//	hexadecimal digits and returns an IntVal or UIntVal built out of
			//	it. The parser is not case-sensitive. The string may or may not
			//	begin with "0x", and other non-hexadecimal characters like white
			//	space, commas, etc. are ignored.
			//
			//	asHex() (defined in subclasses) returns a hexadecimal string
			//	built out of the current integer value. All letter digits will
			//	be lower case. Pass false for the first argument if you don't
			//	want the string to begin with "0x".
			//
			//	The wordSize argument has a bearing on how to pad out the
			//	most-significant bytes. It defaults to the size of the 64-bit
			//	scalar integer type. That means the value 1 will be represented
			//	by "0x0000000000000001", and -1 (for an IntVal) will be
			//	"0xffffffffffffffff".
			//
			//	If the number is larger than what will fit the word size, the
			//	lowest multiple of the word size that fits it will be used
			//	instead. For example, asHex(true, 2) on the value 0x12345 will
			//	become "0x00012345", padding out to 4 bytes.

		 //	static auto FromHex(const HyStr& hex,
		 //		std::size_t wordSize = sizeof(TScalar)
		 //		) -> IntVal/UIntVal;
		 //	auto asHex(
		 //		bool zerox = true, std::size_t wordSize = sizeof(TScalar)
		 //		) const -> std::string;
		};
	constexpr bool kSkipNormalize = false;

	//	When printing an integer value, it will either print the scalar or a
	//	hexadecimal version of the vector, depending on which variant is
	//	available.
	template<typename Child, typename Scalar>
		auto operator<< (
			std::ostream& stream, const IntBase<Child,Scalar>& i
			) -> std::ostream&;

	struct IntVal: IntBase<IntVal, std::int64_t> {
		static auto FromHex(const HyStr& hex,
			std::size_t wordSize = sizeof(TScalar)
			) -> IntVal;
		auto asHex(
			bool zerox = true, std::size_t wordSize = sizeof(TScalar)
			) const -> std::string;

		template<typename, typename> friend struct IntBase;

	#if BINON_CONCEPTS
		IntVal(std::signed_integral auto i): IntBase{i} {}
	#else
		template<
			typename I,
			std::enable_if_t<std::is_integral_v<I> && std::is_signed_v<I>, int>
				= 0
			>
			IntVal(I i): IntBase{i} {}
	#endif
		IntVal(const TVect& v): IntBase{v} {}
		IntVal() noexcept: IntBase{0} {}

		auto asScalar() const noexcept -> TScalar;
		template<typename Int>
			auto fits() const noexcept -> bool;
	};
	struct UIntVal: IntBase<UIntVal, std::uint64_t> {

		static auto FromHex(const HyStr& hex,
			std::size_t wordSize = sizeof(TScalar)
			) -> UIntVal;
		auto asHex(
			bool zerox = true, std::size_t wordSize = sizeof(TScalar)
			) const -> std::string;

		template<typename, typename> friend struct IntBase;

	#if BINON_CONCEPTS
		UIntVal(std::unsigned_integral auto i): IntBase{i} {}
	#else
		template<
			typename I,
			std::enable_if_t<std::is_unsigned_v<I>, int> = 0
			>
			UIntVal(I i): IntBase{i} {}
	#endif
		UIntVal(const TVect& v): IntBase{v} {}
		UIntVal() noexcept: IntBase{0U} {}

		auto asScalar() const noexcept -> TScalar;

		template<typename UInt>
			auto fits() const noexcept -> bool;
	};

	struct IntObj:
		StdAcc<IntObj>,
		StdEq<IntObj>,
		StdHash<IntObj>,
		StdHasDefVal<IntObj>,
		StdPrintArgs<IntObj>,
		StdCodec<IntObj>
	{
		using TValue = IntVal;
		static constexpr auto kTypeCode = kIntObjCode;
		static constexpr auto kClsName = std::string_view{"IntObj"};
		TValue mValue;
		explicit IntObj(const UIntVal& obj);
		IntObj(TValue v);
		IntObj() = default;
		auto operator== (const IntObj& rhs) const noexcept
			{ return equals(rhs); }
		auto operator!= (const IntObj& rhs) const noexcept
			{ return !equals(rhs); }
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const IntObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> IntObj&;
	};
	struct UIntObj:
		StdAcc<UIntObj>,
		StdEq<UIntObj>,
		StdHash<UIntObj>,
		StdHasDefVal<UIntObj>,
		StdPrintArgs<UIntObj>,
		StdCodec<UIntObj>
	{
		using TValue = UIntVal; //std::uint64_t;
		static constexpr auto kTypeCode = kUIntCode;
		static constexpr auto kClsName = std::string_view{"UIntObj"};
		TValue mValue;
		explicit UIntObj(const IntVal& obj);
		UIntObj(TValue v);
		UIntObj() = default;
		auto operator== (const UIntObj& rhs) const noexcept
			{ return equals(rhs); }
		auto operator!= (const UIntObj& rhs) const noexcept
			{ return !equals(rhs); }
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const UIntObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> UIntObj&;
	};

	namespace types {
		using UInt = UIntObj;
	}

	//==== Template Implementation =============================================

	//---- IntBase -------------------------------------------------------------

	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::isScalar(bool norm) const -> bool {
			if(norm) {
				normalize();
			}
			return std::holds_alternative<TScalar>(*this);
		}
	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::scalar(bool norm) const -> TScalar {
			if(!norm) {
				return std::get<TScalar>(*this);
			}
			normalize();
			auto *pScalar = std::get_if<TScalar>(this);
			if(pScalar) {
				return *pScalar;
			}
			throw IntTrunc{"BinON integer is too big to represent in 64 bits"};
		}
	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::vect() -> TVect& {
			return std::get<TVect>(*this);
		}
	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::vect() const -> const TVect& {
			return std::get<TVect>(*this);
		}
	template<typename Child, typename Scalar>
		IntBase<Child,Scalar>::operator TScalar() const {
			return static_cast<const Child*>(this)->scalar();
		}
	template<typename Child, typename Scalar> template<typename N>
		auto IntBase<Child,Scalar>::asNum() const -> N {
			return static_cast<N>(static_cast<const Child*>(this)->asScalar());
		}
	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::asVect() const -> TVect {
			return accessVect<TVect>([](const TVect& v) { return v; });
		}
	template<typename Child, typename Scalar> template<typename ReturnType>
		auto IntBase<Child,Scalar>::accessVect(
			std::function<ReturnType(const TVect&)> callback
			) const -> ReturnType
		{
			using std::get;
			using Callback = std::function<ReturnType(const TVect&)>;
			if(isScalar()) {
				auto i = get<TScalar>(*this);
				TVect v;
				v.reserve(sizeof(TScalar));
				auto n = sizeof(TScalar) << 3;
				while((n -= 8u) > 0u) {
					v.push_back(ToByte(i >> n & 0xff));
				}
				if constexpr(std::is_same_v<ReturnType,void>) {
					callback(v);
				}
				else {
					return callback(v);
				}
			}
			if constexpr(std::is_same_v<ReturnType,void>) {
				callback(get<TVect>(*this));
			}
			else {
				return callback(get<TVect>(*this));
			}
		}
	template<typename Child, typename Scalar>
		void IntBase<Child,Scalar>::normalize(bool shrinkToFit) const
	{
		auto mutThis = const_cast<IntBase*>(this);
		auto pVect = std::get_if<TVect>(mutThis);
		if(!pVect) {
			return; // it's already a scalar
		}
		TVect::size_type trim = 0, sigBytes;
		if constexpr(std::is_unsigned_v<TScalar>) {
			for(auto& byt: *pVect) {

				//	In an unsigned integer, any 0x00 bytes in the most-significant positions can be trimmed off.
				if(byt != 0x00_byte) {
					break;
				}
				++trim;
			}

			//	The number of signficant bytes remaining would simply be the
			//	ones that are not trimmed. (Technically, you could wind up with
			//	0 significant bytes which would be weird, but it doesn't really
			//	matter since anything less than 8 will get the value converted
			//	to scalar.)
			sigBytes = pVect->length() - trim;
		}
		else {

			//	In the signed integer case, the pad bytes could be either a
			//	series of 0x00 or 0xff bytes. We need to track which ones we are
			//	seeing.
			std::byte padByte = 0x00_byte;
			for(auto& byt: *pVect) {
				switch(std::to_integer<unsigned>(byt)) {
				 case 0x00u:
					if(byt != padByte) {
						goto endFor;
					}
					break;
				 case 0xffu:
					if(trim == 0) {
						padByte = 0xff_byte;
					}
					else if(byt != padByte) {
						goto endFor;
					}
				}
				++trim;
			}
		 endFor:
			sigBytes = pVect->length() - trim;

			//	There is a special case for a single 0x00 "pad byte" that should
			//	NOT be trimmed off. When the most-significant bit of the
			//	most-significant byte is set, IntVal treats the number as
			//	negative. In other words, that bit is sign-extended. But say you
			//	wanted to encode 0x80 as the positive number 128 rather than
			//	-128. You would need go with 0x0080 to distinguish it from
			//	0xff80.
			if(trim == 1 && padByte == 0x00_byte) {
				trim = 0;
			}
		}
		if(sigBytes <= sizeof(TScalar)) {
			*mutThis = static_cast<const Child*>(this)->asScalar();
		}
		else if(trim > 0u) {
			pVect->erase(0u, trim);
		}
	}
	template<typename Child, typename Scalar>
		auto operator<< (
			std::ostream& stream, const IntBase<Child,Scalar>& i
			) -> std::ostream&
		{
			auto& child = static_cast<const Child&>(i);
			if(i.isScalar()) {
				stream << child.scalar(kSkipNormalize);
			}
			else {
				stream << child.asHex();
			}
			return stream;
		}

	//---- IntVal --------------------------------------------------------------

	template<typename Int>
		auto IntVal::fits() const noexcept -> bool {
			using std::get;
			if(isScalar()) {
				auto i = scalar(kSkipNormalize);
				if(sizeof(Int) >= sizeof(TScalar)) {
					return true;
				}
				auto limit = 1ULL << (sizeof(Int) * 8 - 1);
				return -limit <= i && i < limit;
			}
			return get<TVect>(*this).size() <= sizeof(Int);
		}

	//---- UIntVal -------------------------------------------------------------

	template<typename UInt>
		auto UIntVal::fits() const noexcept -> bool {
			if(isScalar()) {
				auto i = scalar(kSkipNormalize);
				if(sizeof(UInt) >= sizeof(TScalar)) {
					return true;
				}
				auto limit = 1ULL << sizeof(UInt) << 3;
				return i < limit;
			}
			return std::get<TVect>(*this).size() <= sizeof(UInt);
		}
}

namespace std {
	template<> struct hash<binon::IntVal> {
		auto operator () (const binon::IntVal& i) const noexcept
			-> std::size_t;
	};
	template<> struct hash<binon::UIntVal> {
		auto operator () (const binon::UIntVal& i) const noexcept
			-> std::size_t;
	};
}

#endif
