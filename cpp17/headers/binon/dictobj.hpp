#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "listobj.hpp"

#include <functional>
#include <optional>
#include <sstream>
#include <type_traits>

namespace binon {
	struct TDictObj: TStdCtnr<TDictObj, std::unordered_map<VarObj,VarObj>> {
		using TStdCtnr<TDictObj, std::unordered_map<VarObj,VarObj>>::TStdCtnr;
		static constexpr auto kTypeCode = kDictObjCode;
		static constexpr auto kClsName = std::string_view{"TDictObj"};
		void encodeData(TOStream&, bool requireIO = true) const;
		void decodeData(TIStream&, bool requireIO = true);
		void printArgs(std::ostream&) const;
	};
	struct TSKDict: TStdCtnr<TSKDict, std::unordered_map<VarObj,VarObj>> {
		static constexpr auto kTypeCode = kSKDictCode;
		static constexpr auto kClsName = std::string_view{"TSKDict"};
		CodeByte mKeyCode;
		TSKDict(std::any value, CodeByte keyCode = kIntObjCode);
		TSKDict(CodeByte keyCode = kIntObjCode);
		void encodeData(TOStream& stream, bool requireIO = true) const;
		void decodeData(TIStream& stream, bool requireIO = true);
		void printArgs(std::ostream& stream) const;
	};
	struct TSDict: TStdCtnr<TSDict, std::unordered_map<VarObj,VarObj>> {
		static constexpr auto kTypeCode = kSDictCode;
		static constexpr auto kClsName = std::string_view{"TSDict"};
		CodeByte mKeyCode;
		CodeByte mValCode;
		TSDict(std::any value,
			CodeByte keyCode = kIntObjCode, CodeByte valCode = kIntObjCode
			);
		TSDict(CodeByte keyCode = kIntObjCode, CodeByte valCode = kIntObjCode);
		void encodeData(TOStream& stream, bool requireIO = true) const;
		void decodeData(TIStream& stream, bool requireIO = true);
		void printArgs(std::ostream& stream) const;
	};

	auto DeepCopyTDict(const TDict& dict) -> TDict;
	void PrintTDictRepr(const TDict& list, std::ostream& stream);

	struct NoDictVal: std::logic_error {
		using std::logic_error::logic_error;
	};

	//	Abstract base class of DictObj, SKDict, and SDict that implements some
	//	shared functionality.
	struct DictBase: BinONObj {

		//	Returns the unordered_map (TDict) that stores all the key and value
		//	shared pointers.
		virtual auto dict() noexcept -> TDict& = 0;
		auto dict() const noexcept -> const TDict&
			{ return const_cast<DictBase*>(this)->dict(); }
		operator TDict&() noexcept { return dict(); }
		operator const TDict&() const noexcept { return dict(); }
		template<typename K>
			auto hasValue(K&& key) const -> bool;
		template<typename V, typename K>
			auto findValue(K&& key) const -> std::optional<V>;
		template<typename V, typename K>
			auto getValue(K&& key) const -> V;
		template<typename K, typename V>
			void setValue(K&& key, V&& val);
		template<typename K>
			auto findObjPtr(K&& key) -> TSPBinONObj;
		template<typename K>
			auto findObjPtr(K&& key) const -> const TSPBinONObj;
		template<typename K>
			auto getObjPtr(K&& key) -> TSPBinONObj;
		template<typename K>
			auto getObjPtr(K&& key) const -> const TSPBinONObj;
		template<typename K>
			void setObjPtr(K&& key, TSPBinONObj pObj);

	};

	struct DictObj: DictBase, AccessContainer_mValue<DictObj,TDict> {
		static void EncodeData(
			const TValue& v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		DictObj(const TDict& v): mValue{v} {}
		DictObj(TDict&& v) noexcept: mValue{std::move(v)} {}
		DictObj() noexcept = default;
		explicit operator bool() const noexcept override
			{ return mValue.size() != 0; }
		using DictBase::dict;
		auto dict() noexcept -> TDict& final { return mValue; }
		auto typeCode() const noexcept -> CodeByte final {return kDictObjCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> std::string override
			{ return "DictObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ PrintTDictRepr(mValue, stream); }
	};

	struct SKDictVal {
		CodeByte mKeyCode;
		TDict mDict;
	};
	struct SKDict: DictBase, AccessContainer_mValue<SKDict,SKDictVal> {
		static void EncodeData(
			const TValue& v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		SKDict(CodeByte keyCode = kNullObjCode) noexcept: mValue{keyCode} {}
		SKDict(const SKDictVal& v): mValue{v} {}
		SKDict(SKDictVal&& v) noexcept: mValue{std::move(v)} {}
		SKDict(const SKDict& obj) = default;
		SKDict(SKDict&& obj) noexcept = default;
		explicit operator bool() const noexcept override
			{ return mValue.mDict.size() != 0; }
		using DictBase::dict;
		auto dict() noexcept -> TDict& final { return mValue.mDict; }
		auto typeCode() const noexcept -> CodeByte final {return kSKDictCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> std::string override
			{ return "SKDict"; }
		void printArgsRepr(std::ostream& stream) const override;
	};

	struct SDictVal {
		CodeByte mKeyCode;
		CodeByte mValCode;
		TDict mDict;
	};
	struct SDict: DictBase, AccessContainer_mValue<SDict,SDictVal> {
		static void EncodeData(
			const TValue& v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		SDict(
			CodeByte keyCode = kNullObjCode,
			CodeByte valCode = kNullObjCode
			) noexcept:
			mValue{keyCode, valCode} {}
		SDict(const SDictVal& v): mValue{v} {}
		SDict(SDictVal&& v) noexcept: mValue{std::move(v)} {}
		SDict(const SDict& obj) = default;
		SDict(SDict&& obj) noexcept = default;
		//SDict() noexcept = default;
		auto operator = (const SDict&) -> SDict& = default;
		auto operator = (SDict&&) noexcept -> SDict& = default;
		explicit operator bool() const noexcept override
			{ return mValue.mDict.size() != 0; }
		auto dict() noexcept -> TDict& final { return mValue.mDict; }
		auto typeCode() const noexcept -> CodeByte final {return kSDictCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> std::string override
			{ return "SDict"; }
		void printArgsRepr(std::ostream& stream) const override;
	};


	//==== Template Implementation =============================================

	//---- DictBase ------------------------------------------------------------

	template<typename K>
		auto DictBase::findObjPtr(K&& key) -> TSPBinONObj {
			using std::forward;
			using std::make_shared;
			auto& dct = dict();
			auto pPair = dct.find(make_shared<TWrapper<K>>(forward<K>(key)));
			if(pPair == dct.end()) {
				return {};
			}
			else {
				return pPair->second;
			}
		}
	template<typename K>
		auto DictBase::findObjPtr(K&& key) const -> const TSPBinONObj {
			return const_cast<DictBase*>(this)->findObjPtr(
				std::forward<K>(key)
				);
		}
	template<typename K>
		auto DictBase::getObjPtr(K&& key) -> TSPBinONObj {
			using std::forward;
			using std::make_shared;
			auto pObj = dict().at(make_shared<TWrapper<K>>(forward<K>(key)));
			if(!pObj) {
				throw NullDeref{"unallocated BinON dict value"};
			}
			return pObj;
		}
	template<typename K>
		auto DictBase::getObjPtr(K&& key) const -> const TSPBinONObj {
			return const_cast<DictBase*>(this)->getObjPtr(std::forward<K>(key));
		}
	template<typename K>
		void DictBase::setObjPtr(K&& key, TSPBinONObj pObj) {
			using std::forward;
			using std::make_shared;
			dict().insert_or_assign(
				make_shared<TWrapper<K>>(forward<K>(key)),
				pObj
				);
		}
	template<typename K>
		auto DictBase::hasValue(K&& key) const -> bool {
			return static_cast<bool>(findObjPtr(std::forward<K>(key)));
		}
	template<typename V, typename K>
		auto DictBase::findValue(K&& key) const -> std::optional<V> {
			auto pObj = findObjPtr(std::forward<K>(key));
			if(pObj) {
				return static_cast<V>(SharedObjVal<V>(pObj));
			}
			else  {
				return std::nullopt;
			}
		}
	template<typename V, typename K>
		auto DictBase::getValue(K&& key) const -> V {
			auto pObj = getObjPtr(std::forward<K>(key));
			return static_cast<V>(SharedObjVal<V>(pObj));
		}
	template<typename K, typename V>
		void DictBase::setValue(K&& key, V&& val) {
			using std::forward;
			using std::make_shared;
			dict().insert_or_assign(
				make_shared<TWrapper<K>>(forward<K>(key)),
				make_shared<TWrapper<V>>(forward<V>(val))
				);
		}

}

#endif
