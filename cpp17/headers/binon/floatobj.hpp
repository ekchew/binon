#ifndef BINON_FLOATOBJ_HPP
#define BINON_FLOATOBJ_HPP

#include "binonobj.hpp"

namespace binon {

	struct FloatObj: BinONObj, Access_mValue<FloatObj,types::TFloat64> {
		TValue mValue;

		FloatObj(TValue v=0.0) noexcept: mValue{v} {}
		explicit operator bool() const noexcept override
			{ return mValue != 0; }
		auto typeCode() const noexcept -> CodeByte final {return kFloatObjCode;}
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
		auto clsName() const noexcept -> std::string override
			{ return "FloatObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ stream << mValue; }
	};

	struct Float32Obj: BinONObj, Access_mValue<Float32Obj,types::TFloat32> {
		TValue mValue;

		Float32Obj(TValue v=0.0f) noexcept: mValue{v} {}
		explicit operator bool() const noexcept override
			{ return mValue != 0; }
		auto typeCode() const noexcept -> CodeByte final {return kFloat32Code;}
		void encodeData(TOStream& stream, bool requireIO=true) const final
			{ WriteWord(mValue, stream, requireIO); }
		void decodeData(TIStream& stream, bool requireIO=true) final
		    { mValue = ReadWord<decltype(mValue)>(stream, requireIO); }
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kFloat32Code &&
					*this == static_cast<const Float32Obj&>(other);
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<Float32Obj>(mValue); }
		auto clsName() const noexcept -> std::string override
			{ return "Float32Obj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ stream << mValue; }
	};

	namespace types {
		using Float32 = Float32Obj;
	}
}

namespace std {
	template<> struct hash<binon::FloatObj> {
		constexpr auto operator () (const binon::FloatObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
	template<> struct hash<binon::Float32Obj> {
		constexpr auto operator () (const binon::Float32Obj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
