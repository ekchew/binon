#include "binon/hashutil.hpp"
#include <chrono>
#include <random>

namespace binon {
	using std::size_t;
	using std::uniform_int_distribution;
	std::size_t gHashSalt = [] {
		try {
			//	Try generating the salt using random_device: a source of system
			//	entropy.
			std::random_device rd;
			uniform_int_distribution<size_t> dist1(0);
			return dist1(rd);
		}
		catch(std::exception&) {
			//	Apparently, it is conceivable that random_device may fail and
			//	raise an exception. In that case, we generate the salt using the
			//	clock.
			auto now = std::chrono::high_resolution_clock::now();
			auto seed = now.time_since_epoch().count();

			//	The clock time may have a bunch of zeros in its most-significant
			//	bits. Let's run it through a linear congruential generator to
			//	spread the entropy out a bit. (I know. LCGs suck. But they
			//	should serve fine for this limited purpose I think?)
			std::minstd_rand lcg(
				static_cast<std::minstd_rand::result_type>(seed)
			);
			uniform_int_distribution<size_t> dist2(0);
			return dist2(lcg);
		}

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
