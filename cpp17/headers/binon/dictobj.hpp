#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "binonobj.hpp"

namespace binon {

	class DictObj:
		public BinONObj,
		public AccessContainer_mValue<DictObj,TDict>
	{
	public:
		TValue mValue;
		DictObj(const TValue& v);
		DictObj(TValue&& v) noexcept:
			BinONObj{v.size() == 0}, mValue{std::move(v)} {}
		DictObj(const DictObj& v): DictObj{v.mValue} {}
		DictObj(DictObj&& v) noexcept: DictObj{std::move(v.mValue)} {}
		DictObj() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final {return kDictObjCode;}
		auto makeCopy() const -> TUPBinONObj override
			{ return std::make_unique<DictObj>(mValue); }
	};

}

#endif
