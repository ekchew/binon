#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "binonobj.hpp"

#include <mutex>

namespace binon {
	
	auto DeepCopyTDict(const TDict& dict) -> TDict;
	void PrintTDictRepr(const TDict& list, std::ostream& stream);
	
	struct DictObj: BinONObj, AccessContainer_mValue<DictObj,TDict> {
		TValue mValue;
		
		DictObj(const TDict& v): mValue{v} {}
		DictObj(TDict&& v) noexcept: mValue{std::move(v)} {}
		DictObj(const DictObj& obj) = default;
		DictObj(DictObj&& obj) noexcept = default;
		DictObj() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final {return kDictObjCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto hasDefVal() const -> bool final {return mValue.size() == 0;}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> const char* override
			{ return "DictObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ PrintTDictRepr(mValue); }
	};
	
	struct SKDictVal {
		CodeByte mKeyCode = kIntObjCode;
		TDict mDict;
	};
	struct SKDict: BinONObj, AccessContainer_mValue<SKDict,SKDictVal> {
		TValue mValue;
		
		SKDict(CodeByte keyCode) noexcept: mValue{keyCode} {}
		SKDict(const SKDictVal& v): mValue{v} {}
		SKDict(SKDictVal&& v) noexcept: mValue{std::move(v)} {}
		SKDict(const SKDict& obj) = default;
		SKDict(SKDict&& obj) noexcept = default;
		SKDict() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final {return kSKDictCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto hasDefVal() const -> bool final {return mValue.mDict.size() == 0;}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> const char* override
			{ return "SKDict"; }
		void printArgsRepr(std::ostream& stream) const override;
	};

	struct SDictVal {
		CodeByte mKeyCode = kIntObjCode;
		CodeByte mValCode = kIntObjCode;
		TDict mDict;
	};
	struct SDict: BinONObj, AccessContainer_mValue<SDict,SDictVal> {
		TValue mValue;
		
		SDict(CodeByte keyCode, CodeByte valCode=kIntObjCode) noexcept:
			mValue{keyCode, valCode} {}
		SDict(const SDictVal& v): mValue{v} {}
		SDict(SDictVal&& v) noexcept: mValue{std::move(v)} {}
		SDict(const SDict& obj) = default;
		SDict(SDict&& obj) noexcept = default;
		SDict() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final {return kSDictCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto hasDefVal() const -> bool final {return mValue.mDict.size() == 0;}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept override -> const char* 
			{ return "SDict"; }
		void printArgsRepr(std::ostream& stream) const override;
	};
}

#endif
