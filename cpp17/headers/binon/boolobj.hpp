#ifndef BINON_BOOLOBJ_HPP
#define BINON_BOOLOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	
	class BoolObj: public BinONObj {
	public:
		BoolObj(bool v=false) noexcept: BinONObj{!v}, mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kBoolObjCode;}
		auto getBool() const -> bool final {return mValue;}
		void setBool(bool v) final {mValue = v;}
		void encodeData(OStream& stream, bool requireIO=true) final;
		void decodeData(IStream& stream, bool requireIO=true) final;
	
	private:
		bool mValue;
	};
	
	class TrueObj: public BinONObj {
	public:
		auto typeCode() const noexcept -> CodeByte final
			{ return kTrueObjCode; }
		auto getBool() const -> bool final {return true;}
		void setBool(bool value) final {if(!value) typeErr();}
	};

}

#endif
