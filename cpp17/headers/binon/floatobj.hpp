#ifndef BINON_FLOATOBJ_HPP
#define BINON_FLOATOBJ_HPP

#include "binonobj.hpp"

namespace binon {

	class FloatObj: public BinONObj {
	public:
		FloatObj(TFloat64 v=0.0) noexcept: BinONObj{v == 0.0}, mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kIntObjCode;}
		auto getFloat64() const -> TFloat64 final {return mValue;}
		void setFloat64(TFloat64 v) final {mValue = v;}
		auto getFloat32() const -> TFloat32 final
			{ return static_cast<TFloat32>(mValue); }
		void setFloat32(TFloat32 v) final {mValue = v;}
		void encodeData(OStream& stream, bool requireIO=true) final
			{ WriteWord(mValue, stream, requireIO); }
		void decodeData(IStream& stream, bool requireIO=true) final
		    { mValue = ReadWord<decltype(mValue)>(stream, requireIO); }
	
	private:
		TFloat64 mValue;
	};
	
	class Float32: public BinONObj {
	public:
		Float32(TFloat32 v=0.0f) noexcept: BinONObj{v == 0.0f}, mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kUIntCode;}
		auto getFloat32() const -> TFloat32 final {return mValue;}
		void setFloat32(TFloat32 v) final {mValue = v;}
		void encodeData(OStream& stream, bool requireIO=true) final
			{ WriteWord(mValue, stream, requireIO); }
		void decodeData(IStream& stream, bool requireIO=true) final
		    { mValue = ReadWord<decltype(mValue)>(stream, requireIO); }
	
	private:
		TFloat32 mValue;
	};

}

#endif
