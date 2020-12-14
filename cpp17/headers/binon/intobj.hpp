#ifndef BINON_INTOBJ_HPP
#define BINON_INTOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	
	class IntRangeError: public std::range_error {
	public:
		IntRangeError();
	};
	
	class IntObj: public BinONObj, public Access_mValue<IntObj,std::int64_t>
	{
	public:
		TValue mValue;
		
		IntObj(TValue v=0) noexcept: BinONObj{v == 0}, mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kIntObjCode;}
		auto getInt64() const -> TValue final {return mValue;}
		void setInt64(TValue v) final {mValue = v;}
		auto getUInt64() const -> std::uint64_t final
			{ return static_cast<std::uint64_t>(mValue); }
		void setUInt64(std::uint64_t v) final
			{ mValue = static_cast<TValue>(v); }
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kIntObjCode &&
					*this == static_cast<const IntObj&>(other);
			}
		auto makeCopy() const -> TUPBinONObj override
			{ return std::make_unique<IntObj>(mValue); }
	};
	
	class UInt: public BinONObj, public Access_mValue<UInt,std::uint64_t> {
	public:
		TValue mValue;
		
		UInt(TValue v=0) noexcept: BinONObj{v == 0}, mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kUIntCode;}
		auto getUInt64() const -> TValue final {return mValue;}
		void setUInt64(TValue v) final {mValue = v;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kUIntCode &&
					*this == static_cast<const UInt&>(other);
			}
		auto makeCopy() const -> TUPBinONObj override
			{ return std::make_unique<UInt>(mValue); }
	};

}

namespace std {
	template<> struct hash<binon::IntObj> {
		constexpr auto operator () (const binon::IntObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
	template<> struct hash<binon::UInt> {
		 constexpr auto operator () (const binon::UInt& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
