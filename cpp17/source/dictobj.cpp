#include "binon/dictobj.hpp"

namespace binon {

	/*
	//---- DictObjVal ----------------------------------------------------------
	
	DictObjVal::DictObjVal(const TDict& dict, bool useMutex):
		mDict{dict},
		mUseMutex{useMutex}
	{
	}
	DictObjVal::DictObjVal(TDict&& dict, bool useMutex) noexcept:
		mDict{std::move(dict)},
		mUseMutex{useMutex}
	{
	}
	DictObjVal::DictObjVal(bool useMutex) noexcept:
		mUseMutex{useMutex}
	{
	}
	DictObjVal::DictObjVal(const DictObjVal& val):
		mDict{val.mDict},
		mUseMutex{val.mUseMutex}
	{
	}
	DictObjVal::DictObjVal(DictObjVal&& val) noexcept:
		mDict{std::move(val.mDict)},
		mUseMutex{val.mUseMutex}
	{
	}
	auto DictObjVal::operator = (DictObjVal val) -> DictObjVal& {
		std::swap(mDict, val.mDict);
		mUseMutex = val.mUseMutex;
		return *this;
	}
	auto DictObjVal::deepCopy() const -> DictObjVal {
		TDict dict;
		for(auto&& pair: mDict) {
			dict[pair.first->makeCopy(kDeepCopy)]
				= pair.second->makeCopy(kDeepCopy);
		}
		return DictObjVal{std::move(dict), mUseMutex};
	}
	DictObjVal::~DictObjVal() {
	}
	
	//---- DictObj -------------------------------------------------------------
	
	DictObj::DictObj(const DictObjVal& v):
		mValue{v}
	{
	}
	DictObj::DictObj(DictObjVal&& v) noexcept:
		mValue{std::move(v)}
	{
	}
	auto DictObj::typeCode() const noexcept -> CodeByte {
		return kDictObjCode;
	}
	auto DictObj::hasDefVal() const -> bool {
		return mValue.mDict.size() == 0;
	}
	auto DictObj::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<DictObj>(std::move(mValue.deepCopy()));
		}
		return std::make_shared<DictObj>(*this);
	}
	*/
}
