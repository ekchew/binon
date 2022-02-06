#include "binon/objhelpers.hpp"

namespace binon {
	auto MakeObj(const char* s) -> StrObj {
		return HyStr{s};
	}
	ObjWrapper::ObjWrapper(const BinONObj& obj):
		BinONObj{obj}
	{
	}
	ObjWrapper::ObjWrapper(BinONObj&& obj) noexcept:
		BinONObj{std::forward<BinONObj>(obj)}
	{
	}
	ObjWrapper::ObjWrapper(const char* cStr):
		BinONObj{StrObj{cStr}}
	{
	}
}
