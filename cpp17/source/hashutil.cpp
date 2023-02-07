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

	void CommutativeHash::extend(std::size_t hashVal) {
		mHash ^= (hashVal ^ (hashVal << 16) ^ 89869747UL) * 3644798167UL;
	}
	auto CommutativeHash::get() const -> std::size_t {
		auto h = mHash;
		h = h * 69069U + 907133923UL;

		//	I think this final step in the frozenset hash is only necessary
		//	because their implementation uses -1 as a flag to indicate whether
		//	or not a hash has already been calculated? So I am omitting it here
		//	but leaving it commented out in case there is more to it than I am
		//	aware of.
	 #if 0
		if(h == -1) {
			h = 590923713UL;
		}
	 #endif

		return h;
	}
}
