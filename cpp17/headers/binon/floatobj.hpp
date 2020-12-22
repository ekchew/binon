#ifndef BINON_FLOATOBJ_HPP
#define BINON_FLOATOBJ_HPP

#include "binonobj.hpp"

namespace binon {

	class FloatObj: public BinONObj, public Access_mValue<FloatObj,TFloat64>
	{
	public:
		TValue mValue;
		
		FloatObj(TValue v=0.0) noexcept: mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kFloatObjCode;}
		auto getFloat64() const -> TValue final {return mValue;}
		void setFloat64(TValue v) final {mValue = v;}
		auto getFloat32() const -> TFloat32 final
			{ return static_cast<TFloat32>(mValue); }
		void setFloat32(TFloat32 v) final {mValue = v;}
		void encodeData(TOStream& stream, bool requireIO=true) const final
			{ WriteWord(mValue, stream, requireIO); }
		void decodeData(TIStream& stream, bool requireIO=true) final
		    { mValue = ReadWord<decltype(mValue)>(stream, requireIO); }
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kFloatObjCode &&
					*this == static_cast<const FloatObj&>(other);
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<FloatObj>(mValue); }
		auto hasDefVal() const -> bool final { return mValue == 0; }
	};
	
	class Float32: public BinONObj, public Access_mValue<Float32,TFloat32> {
	public:
		TValue mValue;
		
		Float32(TValue v=0.0f) noexcept: mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kFloat32Code;}
		auto getFloat32() const -> TValue final {return mValue;}
		void setFloat32(TValue v) final {mValue = v;}
		void encodeData(TOStream& stream, bool requireIO=true) const final
			{ WriteWord(mValue, stream, requireIO); }
		void decodeData(TIStream& stream, bool requireIO=true) final
		    { mValue = ReadWord<decltype(mValue)>(stream, requireIO); }
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kFloat32Code &&
					*this == static_cast<const Float32&>(other);
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<Float32>(mValue); }
		auto hasDefVal() const -> bool final { return mValue == 0; }
	};

}

namespace std {
	template<> struct hash<binon::FloatObj> {
		constexpr auto operator () (const binon::FloatObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
	template<> struct hash<binon::Float32> {
		constexpr auto operator () (const binon::Float32& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
