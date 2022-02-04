#include "binon/objhelpers.hpp"

namespace binon {
	auto MakeBinONObj(const char* s) -> BinONObj {
		return StrObj(s);
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
