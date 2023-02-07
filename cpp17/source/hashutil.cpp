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
}
