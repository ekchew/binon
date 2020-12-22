#ifndef BINON_BINON_HPP
#define BINON_BINON_HPP

//	This header includes all the others.
//
//	It's useful to have the makefile which builds the binon library touch this
//	file whenever any of the other headers change. That way, makefiles for
//	other project using the library need only list binon.hpp as a dependency.

#include "binonobj.hpp"
#include "boolobj.hpp"
#include "bufferobj.hpp"
#include "byteutil.hpp"
#include "crtp.hpp"
#include "codebyte.hpp"
#include "dictobj.hpp"
#include "floatobj.hpp"
#include "floattypes.hpp"
#include "intobj.hpp"
#include "listobj.hpp"
#include "literals.hpp"
#include "macros.hpp"
#include "nullobj.hpp"
#include "strobj.hpp"

#endif
