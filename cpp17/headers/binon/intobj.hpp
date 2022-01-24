#ifndef BINON_INTOBJ_HPP
#define BINON_INTOBJ_HPP

#include "binonobj.hpp"
#include "hystr.hpp"
#include <ostream>
#include <type_traits>
#include <variant>

namespace binon {

	template<typename Child, typename Scalar>
		struct IntBase: std::variant<Scalar, std::basic_string<std::byte>>
		{
			using TScalar = Scalar;
			using TVect = std::basic_string<std::byte>;

			using std::variant<TScalar,TVect>::variant;

			auto isScalar() const noexcept -> bool;
			auto canBeScalar() const noexcept -> bool;

			auto scalar() -> TScalar&;
			auto scalar() const -> TScalar;
			auto vect() -> TVect&;
			auto vect() const -> const TVect&;
			operator TScalar() const;
			auto asVect() const -> TVect;
			template<typename ReturnType>
				auto asVect(
					std::function<ReturnType(const TVect&)> callback
					) const -> ReturnType;

			template<typename T>
				auto as() const -> T;
		};
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
		using IntBase<IntVal,TScalar>::IntBase;

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
		using IntBase<UIntVal,TScalar>::IntBase;
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
		using TValue = IntVal;
		static constexpr auto kTypeCode = kIntObjCode;
		static constexpr auto kClsName = std::string_view{"TIntObj"};
		TValue mValue;
		TIntObj(TValue v);
		TIntObj() = default;
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
		using TValue = UIntVal; //std::uint64_t;
		static constexpr auto kTypeCode = kUIntCode;
		static constexpr auto kClsName = std::string_view{"TUIntObj"};
		TValue mValue;
		TUIntObj(TValue v);
		TUIntObj() = default;
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

	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::isScalar() const noexcept -> bool {
			return std::holds_alternative<TScalar>(*this);
		}
	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::canBeScalar() const noexcept -> bool {
			if(isScalar()) {
				return true;
			}
			return std::get<TVect>(*this).size() <= sizeof(TScalar);
		}
	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::scalar() -> TScalar& {
			if(	!isScalar() &&
				std::get<TVect>(*this).size() <= sizeof(TScalar)
				)
			{
				*this = static_cast<Child*>(this)->asScalar();
			}
			return std::get<TScalar>(*this);
		}
	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::scalar() const -> TScalar {
			return const_cast<IntBase*>(this)->scalar();
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
	template<typename Child, typename Scalar> template<typename T>
		auto IntBase<Child,Scalar>::as() const -> T {
			return static_cast<T>(static_cast<const Child*>(this)->asScalar());
		}
	template<typename Child, typename Scalar>
		auto IntBase<Child,Scalar>::asVect() const -> TVect {
			return asVect<TVect>([](const TVect& v) { return v; });
		}
	template<typename Child, typename Scalar> template<typename ReturnType>
		auto IntBase<Child,Scalar>::asVect(
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
			std::ostream& stream, const IntBase<Child,Scalar>& i
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

	//---- IntVal --------------------------------------------------------------

	template<typename Int>
		auto IntVal::fits() const noexcept -> bool {
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

	//---- UIntVal -------------------------------------------------------------

	template<typename UInt>
		auto UIntVal::fits() const noexcept -> bool {
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
	template<> struct hash<binon::IntVal> {
		auto operator () (const binon::IntVal& i) const noexcept
			-> std::size_t;
	};
	template<> struct hash<binon::UIntVal> {
		auto operator () (const binon::UIntVal& i) const noexcept
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
