#include "binon/listhelpers.hpp"

namespace binon {
	auto MakeListObj(std::initializer_list<ObjWrapper> vals) -> ListObj {
		ListObj list;
		auto& stdList = list.value();
		stdList.reserve(vals.size());
		for(auto& v: vals) {
			stdList.push_back(std::move(v));
		}
		return list;
	}
	auto MakeSList(CodeByte elemCode, std::initializer_list<ObjWrapper> vals)
		-> SList
	{
		SList list(elemCode);
		auto& stdList = list.value();
		stdList.reserve(vals.size());
		for(auto& v: vals) {
			stdList.push_back(v.asTypeCodeObj(elemCode));
		}
		return list;
	}
}
