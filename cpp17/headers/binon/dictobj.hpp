#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "binonobj.hpp"

#include <mutex>

namespace binon {
	
	auto DeepCopyTDict(const TDict& dict) -> TDict;
	
	struct DictObj: BinONObj, AccessContainer_mValue<DictObj,TDict> {
		TValue mValue;
		
		DictObj(const TDict& v): mValue{v} {}
		DictObj(TDict&& v) noexcept: mValue{std::move(v)} {}
		DictObj(const DictObj& obj) = default;
		DictObj(DictObj&& obj) noexcept = default;
		DictObj() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final;
		auto hasDefVal() const -> bool final;
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
	};
	
	/*enum: bool { kSkipMutex, kUseMutex };
	
	struct DictObjVal {
		TDict mDict;
		bool mUseMutex;
		mutable std::recursive_mutex mMutex;
		
		DictObjVal(const TDict& dict, bool useMutex=BINON_THREAD_SAFE);
		DictObjVal(TDict&& dict, bool useMutex=BINON_THREAD_SAFE) noexcept;
		DictObjVal(bool useMutex=BINON_THREAD_SAFE) noexcept;
		DictObjVal(const DictObjVal& val);
		DictObjVal(DictObjVal&& val) noexcept;
		auto operator = (DictObjVal val) -> DictObjVal&;
		virtual auto deepCopy() const -> DictObjVal;
		virtual ~DictObjVal();
	};
	struct DictObj: BinONObj, AccessContainer_mValue<DictObj,DictObjVal> {
		TValue mValue;
		
		DictObj(const DictObjVal& v);
		DictObj(DictObjVal&& v) noexcept;
		DictObj(const DictObj& obj) = default;
		DictObj(DictObj&& obj) noexcept = default;
		auto typeCode() const noexcept -> CodeByte final;
		auto hasDefVal() const -> bool final;
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
	};
	
	template<typename Val, typename Fn, typename Res=void, typename... Args>
		BINON_IF_CONCEPTS(requires
			std::derived_from<Val BINON_COMMA DictObjVal>)
		auto AccessValDict(Val& val, Fn fn, Args&&... args) -> Res {
			if(val.mUseMutex) {
				std::lock_guard<decltype(val.mMutex)> lock{val.mMutex};
				return fn(val, std::forward<Args>(args)...);
			}
			return fn(val, std::forward<Args>(args)...);
		}
	*/
		
}

#endif
