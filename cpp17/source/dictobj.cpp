#include "binon/dictobj.hpp"

namespace binon {

	DictObj::DictObj(const TValue& v): BinONObj{v.size() == 0} {
		/*std::transform(BINON_PAR_UNSEQ
			v.begin(), v.end(), mValue.begin(),
			[](const TValue::value_type& p) {
					return
						p->first.makeCopy();
				}
			);*/
	}

}
