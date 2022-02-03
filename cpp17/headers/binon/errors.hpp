#ifndef BINON_ERRORS_HPP
#define BINON_ERRORS_HPP

#include <stdexcept>

namespace binon {

	//	The NoComparing and/or NoHashing exceptions may come up when trying to
	//	use a container type as a dictionary key. This is currently not
	//	supported.
	struct NoComparing: std::invalid_argument {
		using std::invalid_argument::invalid_argument;
	};
	struct NoHashing: std::invalid_argument {
		using std::invalid_argument::invalid_argument;
	};

	//	NullDeref indicates at attempt to dereference a null pointer (including
	//	smart pointers such as std::unique_ptr and std::shared_ptr). This error
	//	used to come up in the binon library back when objects were all
	//	dynamically allocated. The newer implentation uses std::variant to
	//	manage BinON objects, so NullDeref no longer comes up. But the error
	//	type will be left in because it may have future use potential.
	struct NullDeref: std::out_of_range {
		using std::out_of_range::out_of_range;
	};

	//	TypeErr is thrown in the following circumstances:
	//		1. You forget to set the type code(s) for simple list or dictionary
	//		   objects before you encode them.
	//		2. The type codes of elements within simple containers do not match
	//		   the specified code. (Again, this is checked at encoding time.)
	//		3. The various functions in typeconv.hpp cannot ascertain the BinON
	//		   object type corresponding to the general type you supply.
	//		4. You initialize a list or dictionary object with something other
	//		   than a TList or TDict. (This type check cannot be performed at
	//		   compile-time as would normally be the case, so it is done when
	//		   the value() method is called instead. See StdCtnr in mixins.hpp
	//		   for an explanation.)
	struct TypeErr: std::logic_error {
		using std::logic_error::logic_error;
	};
}

#endif
