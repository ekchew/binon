#ifndef BINON_INTOBJ_HPP
#define BINON_INTOBJ_HPP

#include "binonobj.hpp"
#include "hystr.hpp"
#include <ostream>
#include <type_traits>
#include <variant>

namespace binon {

	/*
	Class hierarchy:
		std::variant
			TIntBase:
				TIntVal
				TUIntVal

	TIntVal and TUIntVal store the integer values used by TIntObj and TUIntObj,
	respectively. They do so in a variant record containing either a 64-bit
	C++ integer type or a byte buffer of arbitrary length for really big
	numbers. In the latter case, the bytes are ordered big-endian.
	*/
	template<typename Child, typename Scalar>
		struct TIntBase: std::variant<Scalar, std::basic_string<std::byte>>
		{
			/*
			TIntBase exposes all the functionality (including constructors)
			of the std::variant it inherits from. It also has full access its
			child classes--TIntVal and TUIntVal--through the CRTP paradigm.

			The variant's Scalar type is supplied by the child class in the
			2nd template argument, but is std::int64_t for TIntVal and
			std::uint64_t for TUIntVal.

			The vector type is actually a string of std::byte. (It used to be
			a std::vector<std::byte> but was changed to string due to several
			advantages the latter boasts, like the small string optimization
			and built-in hashing support.)
			*/
			using TScalar = Scalar;
			using TVect = std::basic_string<std::byte>;

			using std::variant<TScalar,TVect>::variant;

			/*
			isScalar() vs. canBeScalar()

			The isScalar() method is a quick way to tell whether the variant
			current holds a scalar or a vector.

			canBeScalar() may return true even in the vector case if the vector
			is short enough to be converted into a 64-bit integer without any
			truncation. It is not a bad idea to call it before calling scalar().
			*/
			auto isScalar() const noexcept -> bool;
			auto canBeScalar() const noexcept -> bool;

			/*
			Scalar access/conversion

			If the variant already holds a scalar, the scalar() method will
			return it right away. Otherwise, it will check if the vector is
			short enough to convert to a 64-bit integer without truncsation.
			If so, scalar() will perform the conversion in-place and return it.
			(Note that this will happen even with the const version of the
			method.) Otherwise, it will throw std::bad_variant_access.

			scalar() is also mapped onto an implicit conversion operator to
			TScalar. This is to help TIntVal and TUIntVal to function in many
			situations where you would want to work with a std::int64_t or
			std::uint64_t directly, but be aware of this potential for an
			exception.

			The asScalar() method (defined in the child classes) will return
			the 64-bit integer without throwing any exceptions. In other words,
			it will truncate if necessary without complaining. There is also
			an asNum<N>() method that casts to any numeric type. asNum<int>(),
			for example, is equivalent to static_cast<int>(asScalar()).
			(asNum<N>() will only throw an exception if N is some weird class
			whose constructor could throw.)
			*/
			auto scalar() -> TScalar&;
			auto scalar() const -> TScalar;
			operator TScalar() const;
			template<typename N>
				auto asNum() const -> N;

			/*
			Vector access/conversion

			vect() is the counterpart to scalar() in that it returns the
			vector variant where available. It will NOT, however, perform an
			in-place conversion from scalar but throw std::bad_variant_access
			instead.

			asVect() will perform the conversion though, throwing an exception
			only if it runs out of memory in allocating the vector. This method
			comes in two forms. The simpler is much like asScalar(). It simply
			returns the vector by value. The lower-level form accepts a
			callback functor you would typically implement using a lambda
			expression. The callback accespts a single argument--a constant
			reference to the vector--and may return a type you supply. This
			will then get returned by the asVect() method itself. The advantage
			of the callback approach is that it can be a bit more efficient
			when the variant already contains a vector, since it can pass a
			reference to it rather than returning a copy.
			*/
			auto vect() -> TVect&;
			auto vect() const -> const TVect&;
			auto asVect() const -> TVect;
			template<typename ReturnType>
				auto asVect(
					std::function<ReturnType(const TVect&)> callback
					) const -> ReturnType;

		};

	/*
	When printing an integer value, it will either print the scalar or a
	hexadecimal version of the vector, depending on which variant is available.
	*/
	template<typename Child, typename Scalar>
		auto operator<< (
			std::ostream& stream, const TIntBase<Child,Scalar>& i
			) -> std::ostream&;

	struct TIntVal: TIntBase<TIntVal, std::int64_t> {

		/*
		You can convert both TIntVal and TUIntVal into a hexadecimal string by
		calling asHex(), or build either object by calling the FromHex()
		class method.

		FromHex() accepts a HyStr--or hybrid string--which can be either a
		std::string or a std::string_view (see hystr.hpp). The hexadecimal may
		or may not begin with a "0x". FromHex() does not care.
		Any non-hexadecimal characters are ignored. Both lower and upper case
		are fine for any letter digits.

		asHex()'s first argument--zerox--determines whether it should insert
		"0x" at the beginning of the string it returns. All output is lower case
		with no non-hexadecimal characters (beyond the 0x).

		Both methods have an optional second argument: the word size. This has
		a bearing on how many pad bytes (if any) to insert ahead of the data.
		The default 16 (the length of a 64-bit scalar) means a number like
		0x123abc will be printed "0x0000000000123abc". The pad bytes can be
		especially important for TIntVal which support negative numbers. In this
		case, they will be "ff" rather than "00". If the number of significant
		bytes exceeds the word size, the padding will be extended to the next
		word boundary. For example, if your integer were 0x12345 and your
		word size was only 4, you would get 0x00012345 (in other words, 2
		4-byte words).
		*/
		static auto FromHex(const HyStr& hex,
			std::size_t wordSize = sizeof(TScalar)
			) -> TIntVal;
		auto asHex(
			bool zerox = true, std::size_t wordSize = sizeof(TScalar)
			) const -> std::string;

		template<typename, typename> friend struct TIntBase;
		using TIntBase<TIntVal,TScalar>::TIntBase;

		auto asScalar() const noexcept -> TScalar;

		/*
		The fits() method determines whether the current integer would fit an
		integer type you specify without truncation. It can be useful to call
		fits<int>() before asNum<int>(), for example.
		*/
		template<typename Int>
			auto fits() const noexcept -> bool;
	};
	struct TUIntVal: TIntBase<TUIntVal, std::uint64_t> {

		static auto FromHex(const HyStr& hex,
			std::size_t wordSize = sizeof(TScalar)
			) -> TUIntVal;
		auto asHex(
			bool zerox = true, std::size_t wordSize = sizeof(TScalar)
			) const -> std::string;

		template<typename, typename> friend struct TIntBase;
		using TIntBase<TUIntVal,TScalar>::TIntBase;
		auto asScalar() const noexcept -> TScalar;

		template<typename UInt>
			auto fits() const noexcept -> bool;
	};

	struct TIntObj:
		TStdAcc<TIntObj>,
		TStdEq<TIntObj>,
		TStdHash<TIntObj>,
		TStdHasDefVal<TIntObj>,
		TStdPrintArgs<TIntObj>,
		TStdCodec<TIntObj>
	{
		using TValue = TIntVal;
		static constexpr auto kTypeCode = kIntObjCode;
		static constexpr auto kClsName = std::string_view{"TIntObj"};
		TValue mValue;
		TIntObj(TValue v);
		TIntObj() = default;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TIntObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TIntObj&;
	};
	struct TUIntObj:
		TStdAcc<TUIntObj>,
		TStdEq<TUIntObj>,
		TStdHash<TUIntObj>,
		TStdHasDefVal<TUIntObj>,
		TStdPrintArgs<TUIntObj>,
		TStdCodec<TUIntObj>
	{
		using TValue = TUIntVal; //std::uint64_t;
		static constexpr auto kTypeCode = kUIntCode;
		static constexpr auto kClsName = std::string_view{"TUIntObj"};
		TValue mValue;
		TUIntObj(TValue v);
		TUIntObj() = default;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TUIntObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TUIntObj&;
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

	//---- TIntBase -------------------------------------------------------------

	template<typename Child, typename Scalar>
		auto TIntBase<Child,Scalar>::isScalar() const noexcept -> bool {
			return std::holds_alternative<TScalar>(*this);
		}
	template<typename Child, typename Scalar>
		auto TIntBase<Child,Scalar>::canBeScalar() const noexcept -> bool {
			if(isScalar()) {
				return true;
			}
			return std::get<TVect>(*this).size() <= sizeof(TScalar);
		}
	template<typename Child, typename Scalar>
		auto TIntBase<Child,Scalar>::scalar() -> TScalar& {
			if(	!isScalar() &&
				std::get<TVect>(*this).size() <= sizeof(TScalar)
				)
			{
				*this = static_cast<Child*>(this)->asScalar();
			}
			return std::get<TScalar>(*this);
		}
	template<typename Child, typename Scalar>
		auto TIntBase<Child,Scalar>::scalar() const -> TScalar {
			return const_cast<TIntBase*>(this)->scalar();
		}
	template<typename Child, typename Scalar>
		auto TIntBase<Child,Scalar>::vect() -> TVect& {
			return std::get<TVect>(*this);
		}
	template<typename Child, typename Scalar>
		auto TIntBase<Child,Scalar>::vect() const -> const TVect& {
			return std::get<TVect>(*this);
		}
	template<typename Child, typename Scalar>
		TIntBase<Child,Scalar>::operator TScalar() const {
			return static_cast<const Child*>(this)->scalar();
		}
	template<typename Child, typename Scalar> template<typename N>
		auto TIntBase<Child,Scalar>::asNum() const -> N {
			return static_cast<N>(static_cast<const Child*>(this)->asScalar());
		}
	template<typename Child, typename Scalar>
		auto TIntBase<Child,Scalar>::asVect() const -> TVect {
			return asVect<TVect>([](const TVect& v) { return v; });
		}
	template<typename Child, typename Scalar> template<typename ReturnType>
		auto TIntBase<Child,Scalar>::asVect(
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
				return callback(v);
			}
			return callback(get<TVect>(*this));
		}
	template<typename Child, typename Scalar>
		auto operator<< (
			std::ostream& stream, const TIntBase<Child,Scalar>& i
			) -> std::ostream&
		{
			auto& child = static_cast<const Child&>(i);
			if(i.canBeScalar()) {
				stream << child.scalar();
			}
			else {
				stream << child.asHex();
			}
			return stream;
		}

	//---- TIntVal --------------------------------------------------------------

	template<typename Int>
		auto TIntVal::fits() const noexcept -> bool {
			using std::get;
			if(canBeScalar()) {
				auto i = scalar();
				if(sizeof(Int) >= sizeof(TScalar)) {
					return true;
				}
				auto limit = 1ULL << (sizeof(Int) * 8 - 1);
				return -limit <= i && i < limit;
			}
			return get<TVect>(*this).size() <= sizeof(Int);
		}

	//---- TUIntVal -------------------------------------------------------------

	template<typename UInt>
		auto TUIntVal::fits() const noexcept -> bool {
			if(canBeScalar()) {
				auto i = scalar();
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
	template<> struct hash<binon::TIntVal> {
		auto operator () (const binon::TIntVal& i) const noexcept
			-> std::size_t;
	};
	template<> struct hash<binon::TUIntVal> {
		auto operator () (const binon::TUIntVal& i) const noexcept
			-> std::size_t;
	};
	template<> struct hash<binon::IntObj> {
		auto operator () (const binon::IntObj& obj) const noexcept
			-> std::size_t;
	};
	template<> struct hash<binon::UIntObj> {
		auto operator () (const binon::UIntObj& obj) const noexcept
			-> std::size_t;
	};
}

#endif
