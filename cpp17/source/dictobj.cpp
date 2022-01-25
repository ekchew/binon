#include "binon/dictobj.hpp"
#include "binon/listobj.hpp"
#include "binon/packelems.hpp"
#include "binon/varobj.hpp"

#include <iostream>

namespace binon {

	//---- TDictObj ------------------------------------------------------------

	void TDictObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto& u = value();
		TUIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		for(auto& [k, v]: u) {
			k.encode(stream, kSkipRequireIO);
		}
		for(auto& [k, v]: u) {
			v.encode(stream, kSkipRequireIO);
		}
	}
	void TDictObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto& u = value();
		TUIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		std::vector<TVarObj> ks;
		ks.reserve(n);
		u.clear();
		u.reserve(n);
		while(n-->0) {
			ks.push_back(TVarObj::Decode(stream, kSkipRequireIO));
		}
		for(auto& k: ks) {
			u[std::move(k)] = TVarObj::Decode(stream, kSkipRequireIO);
		}
	}
	void TDictObj::printArgs(std::ostream& stream) const {
		stream << "TDictObj::TValue{";
		auto& u = value();
		bool first = true;
		for(auto& [k, v]: u) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			stream << '{';
			k.print(stream);
			stream << ", ";
			v.print(stream);
			stream << '}';
		}
		stream << "}";
	}

	//---- TSKDict -------------------------------------------------------------

	TSKDict::TSKDict(std::any value, CodeByte keyCode):
		TStdCtnr<TSKDict,TValue>{std::move(value)},
		mKeyCode{keyCode}
	{
	}
	TSKDict::TSKDict(CodeByte keyCode):
		mKeyCode{keyCode}
	{
	}
	void TSKDict::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto& u = value();
		TUIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		mKeyCode.write(stream, kSkipRequireIO);
		{
			PackElems packKey{mKeyCode, stream};
			for(auto& [k, v]: u) {
				packKey(k, kSkipRequireIO);
			}
		}
		for(auto& [k, v]: u) {
			v.encode(stream, kSkipRequireIO);
		}
	}
	void TSKDict::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto& u = value();
		TUIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		mKeyCode = CodeByte::Read(stream, kSkipRequireIO);
		std::vector<TVarObj> ks;
		ks.reserve(n);
		u.clear();
		u.reserve(n);
		UnpackElems unpackKey{mKeyCode, stream};
		while(n-->0) {
			ks.push_back(unpackKey(kSkipRequireIO));
		}
		for(auto& k: ks) {
			u[std::move(k)] = TVarObj::Decode(stream, kSkipRequireIO);
		}
	}
	void TSKDict::printArgs(std::ostream& stream) const {
		stream << "TSKDict::TValue{";
		auto& u = value();
		bool first = true;
		for(auto& [k, v]: u) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			stream << '{';
			k.print(stream);
			stream << ", ";
			v.print(stream);
			stream << '}';
		}
		stream << "}, ";
		mKeyCode.printRepr(stream);
	}

	//---- TSDict -------------------------------------------------------------

	TSDict::TSDict(std::any value, CodeByte keyCode, CodeByte valCode):
		TStdCtnr<TSDict,TValue>{std::move(value)},
		mKeyCode{keyCode},
		mValCode{valCode}
	{
	}
	TSDict::TSDict(CodeByte keyCode, CodeByte valCode):
		mKeyCode{keyCode},
		mValCode{valCode}
	{
	}
	void TSDict::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto& u = value();
		TUIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		mKeyCode.write(stream, kSkipRequireIO);
		{
			PackElems packKey{mKeyCode, stream};
			for(auto& [k, v]: u) {
				packKey(k, kSkipRequireIO);
			}
		}
		mValCode.write(stream, kSkipRequireIO);
		{
			PackElems packVal{mValCode, stream};
			for(auto& [k, v]: u) {
				packVal(v, kSkipRequireIO);
			}
		}
	}
	void TSDict::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto& u = value();
		TUIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		mKeyCode = CodeByte::Read(stream, kSkipRequireIO);
		std::vector<TVarObj> ks;
		ks.reserve(n);
		u.clear();
		u.reserve(n);
		UnpackElems unpackKey{mKeyCode, stream};
		while(n-->0) {
			ks.push_back(unpackKey(kSkipRequireIO));
		}
		mValCode = CodeByte::Read(stream, kSkipRequireIO);
		UnpackElems unpackVal{mValCode, stream};
		for(auto& k: ks) {
			u[std::move(k)] = unpackVal(kSkipRequireIO);
		}
	}
	void TSDict::printArgs(std::ostream& stream) const {
		stream << "TSDict::TValue{";
		auto& u = value();
		bool first = true;
		for(auto& [k, v]: u) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			stream << '{';
			k.print(stream);
			stream << ", ";
			v.print(stream);
			stream << '}';
		}
		stream << "}, ";
		mKeyCode.printRepr(stream);
		stream << ", ";
		mValCode.printRepr(stream);
	}

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
