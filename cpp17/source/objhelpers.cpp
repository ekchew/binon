#include "binon/objhelpers.hpp"

namespace binon {
	auto MakeBinONObj(const char* s) -> BinONObj {
		return StrObj(s);
	}
}
