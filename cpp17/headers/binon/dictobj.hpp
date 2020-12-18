#ifndef BINON_DICTOBJ_HPP
#define BINON_DICTOBJ_HPP

#include "binonobj.hpp"

namespace binon {

	class DictObj:
		public BinONObj,
		public AccessContainer_mValue<ListObj,TDict>
	{
	};

}

#endif
