#ifndef BINON_BINON_HPP
#define BINON_BINON_HPP

//	This header includes all the others. It is also touched if any of the others
//	are altered and binon is rebuilt. That way, you can make this one header a
//	dependency for any other projects that use libbinon.

#include "binonobj.hpp"
#include "boolobj.hpp"
#include "bufferobj.hpp"
#include "byteutil.hpp"
#include "codebyte.hpp"
#include "dicthelpers.hpp"
#include "dictobj.hpp"
#include "errors.hpp"
#include "floatobj.hpp"
#include "floattypes.hpp"
#include "hystr.hpp"
#include "idgen.hpp"
#include "intobj.hpp"
#include "iterable.hpp"
#include "listhelpers.hpp"
#include "listobj.hpp"
#include "literals.hpp"
#include "macros.hpp"
#include "nullobj.hpp"
#include "objhelpers.hpp"
#include "optutil.hpp"
#include "strobj.hpp"
#include "typeconv.hpp"
#include "typeutil.hpp"

#endif
