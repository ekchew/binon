#include "binon/dictobj.hpp"
#include "binon/listobj.hpp"
#include "binon/packelems.hpp"
#include "binon/varobj.hpp"

#include <iostream>

namespace binon {

	//---- TDictBase -----------------------------------------------------------

	/*TDictBase::TDictBase(const TValue& value):
		mValue{value}
	{
	}
	TDictBase::TDictBase(TValue&& value):
		mValue{std::forward<TValue>(value)}
	{
	}
	TDictBase::TDictBase():
		mValue{TValue()}
	{
	}
	auto TDictBase::value() -> TValue& {
		return std::any_cast<TValue&>(mValue);
	}
	auto TDictBase::value() const -> const TValue& {
		return std::any_cast<const TValue&>(mValue);
	}
	auto TDictBase::hasDefVal() const -> bool {
		return value().size() == 0;
	}*/

	//--------------------------------------------------------------------------

	auto DeepCopyTDict(const TDict& dict) -> TDict {
		TDict copy;
		for(auto&& pair: dict) {
			copy[pair.first->makeCopy(kDeepCopy)]
				= pair.second->makeCopy(kDeepCopy);
		}
		return std::move(copy);
	}
	void PrintTDictRepr(const TDict& list, std::ostream& stream) {
		stream << "TDict{";
		bool first = true;
		for(auto&& pair: list) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			stream << '{';
			pair.first->printPtrRepr(stream);
			stream << ", ";
			pair.second->printPtrRepr(stream);
			stream << '}';
		}
		stream << '}';
	}

	//---- DictBase ------------------------------------------------------------

	#if 0
	auto DictBase::hasKey(const TSPBinONObj& pKey) const -> bool {
		const TDict& dct = dict();
		return dct.find(pKey) != dct.end();
	}
	auto DictBase::hasValue(const TSPBinONObj& pKey) const -> bool {
		const TDict& dct = dict();
		auto iter = dct.find(pKey);
		return iter != dct.end() && iter->second;
	}
	#endif

	//---- DictObj -------------------------------------------------------------

	void DictObj::EncodeData(
		const TValue& v, TOStream& stream, bool requireIO)
	{
		using TKey = TSPBinONObj;
		using TVal = TSPBinONObj;
		RequireIO rio{stream, requireIO};
		UIntObj::EncodeData(v.size(), stream, kSkipRequireIO);
		ListObj::EncodeElems(
			PipeGenRefs<TSPBinONObj>(
				IterGen{v.begin(), v.end()},
				[](auto& kv) { return kv.first; }
			), stream, kSkipRequireIO);
		ListObj::EncodeElems(
			PipeGenRefs<TSPBinONObj>(
				IterGen{v.begin(), v.end()},
				[](auto& kv) { return kv.second; }
			), stream, kSkipRequireIO);
	}
	auto DictObj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		TValue v;
		RequireIO rio{stream, requireIO};
		TList keys = ListObj::DecodeData(stream, kSkipRequireIO);
		auto keyIt = keys.begin();
		auto valsGen = ListObj::DecodedElemsGen(
			stream, keys.size(), kSkipRequireIO);
		for(auto&& val: valsGen) {
			v[*keyIt++] = val;
		}
		return std::move(v);
	}
	void DictObj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void DictObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}
	auto DictObj::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<DictObj>(DeepCopyTDict(mValue));
		}
		return std::make_shared<DictObj>(*this);
	}

	//---- SKDict --------------------------------------------------------------

	void SKDict::EncodeData(
		const TValue& v, TOStream& stream, bool requireIO)
	{
		RequireIO rio{stream, requireIO};

		SList keys{SListVal{v.mKeyCode, TList(v.mDict.size())}};
		auto& keyList = keys.mValue.mList;
		keyList.clear();
		for(auto&& [key, val]: v.mDict) {
			keyList.push_back(key);
		}
		keys.encodeData(stream, kSkipRequireIO);

		ListObj::EncodeElems(
			PipeGenRefs<TSPBinONObj>(
				IterGen{v.mDict.begin(), v.mDict.end()},
				[](auto& kv) { return kv.second; }
			), stream, kSkipRequireIO);
	}
	auto SKDict::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		RequireIO rio{stream, requireIO};
		SList keys;
		keys.decodeData(stream, kSkipRequireIO);
		auto& keyList = keys.mValue.mList;
		auto keyIt = keyList.begin();
		TValue v{keys.mValue.mElemCode};
		auto valsGen = ListObj::DecodedElemsGen(
			stream, keyList.size(), kSkipRequireIO);
		for(auto val: valsGen) {
			v.mDict[*keyIt++] = val;
		}
		return std::move(v);
	}
	void SKDict::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void SKDict::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}
	auto SKDict::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<SKDict>(
				SKDictVal{mValue.mKeyCode, DeepCopyTDict(mValue.mDict)}
			);
		}
		return std::make_shared<SKDict>(*this);
	}
	void SKDict::printArgsRepr(std::ostream& stream) const {
		stream << "SKDictVal{";
		mValue.mKeyCode.printRepr(stream);
		stream << ", ";
		PrintTDictRepr(mValue.mDict, stream);
		stream << '}';
	}

	//---- SDict ---------------------------------------------------------------

	void SDict::EncodeData(
		const TValue& v, TOStream& stream, bool requireIO)
	{
		RequireIO rio{stream, requireIO};
		auto n = v.mDict.size();
		SList keys{SListVal{v.mKeyCode, TList(n)}};
		SList vals{SListVal{v.mValCode, TList(n)}};
		auto& keyList = keys.mValue.mList;
		auto& valList = vals.mValue.mList;
		keyList.clear();
		valList.clear();
		for(auto&& [key, val]: v.mDict) {
			keyList.push_back(key);
			valList.push_back(val);
		}
		keys.encodeData(stream, kSkipRequireIO);
		vals.encodeElems(stream, kSkipRequireIO);
	}
	auto SDict::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		RequireIO rio{stream, requireIO};
		SList keys;
		keys.decodeData(stream, kSkipRequireIO);
		TValue v{keys.mValue.mElemCode};
		auto n = keys.mValue.mList.size();
		SList vals{SListVal{kIntObjCode, TList(n)}};
		vals.decodeElems(stream, n, kSkipRequireIO);
		v.mValCode = vals.mValue.mElemCode;
		for(decltype(n) i = 0; i < n; ++i) {
			v.mDict[keys.mValue.mList[i]] = vals.mValue.mList[i];
		}
		return std::move(v);
	}
	void SDict::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void SDict::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}
	auto SDict::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<SDict>(SDictVal{
				mValue.mKeyCode, mValue.mValCode, DeepCopyTDict(mValue.mDict)
			});
		}
		return std::make_shared<SDict>(*this);
	}
	void SDict::printArgsRepr(std::ostream& stream) const {
		stream << "SDictVal{";
		mValue.mKeyCode.printRepr(stream);
		stream << ", ";
		mValue.mValCode.printRepr(stream);
		stream << ", ";
		PrintTDictRepr(mValue.mDict, stream);
		stream << '}';
	}
}
