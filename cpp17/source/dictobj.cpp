#include "binon/dictobj.hpp"
#include "binon/listobj.hpp"

namespace binon {

	auto DeepCopyTDict(const TDict& dict) -> TDict {
		TDict copy;
		for(auto&& pair: dict) {
			copy[pair.first->makeCopy(kDeepCopy)]
				= pair.second->makeCopy(kDeepCopy);
		}
		return std::move(copy);
	}
	void PrintTDictRepr(const TDict& list, std::ostream& stream) {
		stream << "TDict{{";
		bool first = true;
		for(auto&& pair: list) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			stream << '{';
			pair.first->printRepr(stream);
			stream << ", ";
			pair.second->printRepr(stream);
			stream << '}';
		}
		stream << "}}";
	}
	
	//---- DictObj -------------------------------------------------------------
	
	void DictObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto n = mValue.size();
		ListObj keys{TList(n)}, vals{TList(n)};
		decltype(n) i = 0;
		for(auto&& pair: mValue) {
			keys.mValue[i] = pair.first;
			vals.mValue[i] = pair.second;
		}
		keys.encodeData(stream, kSkipRequireIO);
		vals.encodeElems(stream, kSkipRequireIO);
	}
	void DictObj::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		ListObj keys;
		keys.decodeData(stream, kSkipRequireIO);
		auto n = keys.mValue.size();
		ListObj vals{TList(n)};
		vals.decodeElems(stream, n, kSkipRequireIO);
		mValue.clear();
		for(decltype(n) i = 0; i < n; ++i) {
			mValue[keys.mValue[i]] = vals.mValue[i];
		}
	}
	auto DictObj::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<DictObj>(DeepCopyTDict(mValue));
		}
		return std::make_shared<DictObj>(*this);
	}
	void DictObj::printRepr(std::ostream& stream) const {
		stream << "DictObj{";
		PrintTDictRepr(mValue);
		stream << '}';
	}
	
	//---- SKDict --------------------------------------------------------------
	
	void SKDict::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto n = mValue.mDict.size();
		SList keys{SListVal{mValue.mKeyCode, TList(n)}};
		ListObj vals{TList(n)};
		decltype(n) i = 0;
		for(auto&& pair: mValue.mDict) {
			keys.mValue.mList[i] = pair.first;
			vals.mValue[i] = pair.second;
		}
		keys.encodeData(stream, kSkipRequireIO);
		vals.encodeElems(stream, kSkipRequireIO);
	}
	void SKDict::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		SList keys;
		keys.decodeData(stream, kSkipRequireIO);
		mValue.mKeyCode = keys.mValue.mElemCode;
		auto n = keys.mValue.mList.size();
		ListObj vals{TList(n)};
		vals.decodeElems(stream, n, kSkipRequireIO);
		mValue.mDict.clear();
		for(decltype(n) i = 0; i < n; ++i) {
			mValue.mDict[keys.mValue.mList[i]] = vals.mValue[i];
		}
	}
	auto SKDict::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<SKDict>(
				SKDictVal{mValue.mKeyCode, DeepCopyTDict(mValue.mDict)}
			);
		}
		return std::make_shared<SKDict>(*this);
	}	
	void SKDict::printRepr(std::ostream& stream) const {
		stream << "SKDict{SKDictVal{";
		mValue.mKeyCode.printRepr(stream);
		stream <, ", {";
		PrintTDictRepr(mValue.mDict);
		stream << "}}}";
	}
	
	//---- SDict ---------------------------------------------------------------
	
	void SDict::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto n = mValue.mDict.size();
		SList keys{SListVal{mValue.mKeyCode, TList(n)}};
		SList vals{SListVal{mValue.mValCode, TList(n)}};
		decltype(n) i = 0;
		for(auto&& pair: mValue.mDict) {
			keys.mValue.mList[i] = pair.first;
			vals.mValue.mList[i] = pair.second;
		}
		keys.encodeData(stream, kSkipRequireIO);
		vals.encodeElems(stream, kSkipRequireIO);
	}
	void SDict::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		SList keys;
		keys.decodeData(stream, kSkipRequireIO);
		mValue.mKeyCode = keys.mValue.mElemCode;
		auto n = keys.mValue.mList.size();
		SList vals{SListVal{kIntObjCode, TList(n)}};
		vals.decodeElems(stream, n, kSkipRequireIO);
		mValue.mValCode = vals.mValue.mElemCode;
		mValue.mDict.clear();
		for(decltype(n) i = 0; i < n; ++i) {
			mValue.mDict[keys.mValue.mList[i]] = vals.mValue.mList[i];
		}
	}
	auto SDict::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<SDict>(SDictVal{
				mValue.mKeyCode, mValue.mValCode, DeepCopyTDict(mValue.mDict)
			});
		}
		return std::make_shared<SDict>(*this);
	}	
	void SDict::printRepr(std::ostream& stream) const {
		stream << "SDict{SKDictVal{";
		mValue.mKeyCode.printRepr(stream);
		stream <, ", ";
		mValue.mValCode.printRepr(stream);
		stream <, ", {";
		PrintTDictRepr(mValue.mDict);
		stream << "}}}";
	}
}
