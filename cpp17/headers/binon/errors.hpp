#ifndef BINON_ERRORS_HPP
#define BINON_ERRORS_HPP

#include <stdexcept>

namespace binon {
	struct NoComparing: std::invalid_argument {
		using std::invalid_argument::invalid_argument;
	};
	struct NoHashing: std::invalid_argument {
		using std::invalid_argument::invalid_argument;
	};
	struct NullDeref: std::out_of_range {
		using std::out_of_range::out_of_range;
	};
	struct TypeErr: std::logic_error {
		using std::logic_error::logic_error;
	};

}

#endif
