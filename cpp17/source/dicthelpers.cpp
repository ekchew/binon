#include "binon/dicthelpers.hpp"

namespace binon {
	auto MakeDictObj(std::initializer_list<TCTypePair> pairs) -> DictObj {
		DictObj obj;
		auto& map = obj.value();
		for(auto& pair: pairs) {
			map[pair.first] = std::move(pair.second);
		}
		return obj;
	}
	auto MakeSKDict(
		CodeByte keyCode, std::initializer_list<TCTypePair> pairs
	) -> SKDict {
		SKDict obj{keyCode};
		auto& map = obj.value();
		for(auto& pair: pairs) {
			map[pair.first] = std::move(pair.second);
		}
		return obj;
	}
	auto MakeSDict(
		CodeByte keyCode, CodeByte valCode,
		std::initializer_list<TCTypePair> pairs
	) -> SDict {
		SDict obj{keyCode, valCode};
		auto& map = obj.value();
		for(auto& pair: pairs) {
			map[pair.first] = std::move(pair.second);
		}
		return obj;
	}
}
