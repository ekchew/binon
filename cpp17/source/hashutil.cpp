#include "binon/hashutil.hpp"
#include "binon/seedsource.hpp"

namespace binon {
	using std::size_t;
	using std::uniform_int_distribution;
	std::size_t gHashSalt = [] {
		SeedSource::Seq seq;
		std::minstd_rand lcg{seq};
		uniform_int_distribution<size_t> dis;
		return dis(lcg);
	}();
}
