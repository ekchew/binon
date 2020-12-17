#ifndef BINON_FLOATTYPES_HPP
#define BINON_FLOATTYPES_HPP

//	In the absence of a <cstdfloat>, this header attempts to define 32-bit and
//	64-bit IEEE 754 floating-point data types as binon::TFloat32 and
//	binon::TFloat64, respectively.
//	
//	By default, these map onto the built-in float and double types, but there
//	are static assertions to make sure they are the correct number of bytes at
//	least. If one of these assertions fails, you may be able to work around it
//	by defining BINON_FLOAT32 or BINON_FLOAT64 as appropriate in your
//	makefile.
//	
//	For example, say you are targetting a legacy compiler/CPU in which the
//	"double" type is 80 bits but you can define a 64-bit value as "short
//	double". Then you could use the compiler option:
//
//		-D"BINON_FLOAT64=short double"
//
//	Alternatively, you could
//
//		#define BINON_FLOAT64 short double
//
//	somewhere in your own code before including this header.

#include <cfloat>
#include <climits>

static_assert(CHAR_BIT == 8, "binon requires 8-bit bytes");
static_assert(FLT_RADIX == 2, "binon expects IEEE 754 floating-point types");

#ifndef BINON_FLOAT32
	#define BINON_FLOAT32 float
#endif
#ifndef BINON_FLOAT64
	#define BINON_FLOAT64 double
#endif

namespace binon {
	using TFloat32 = BINON_FLOAT32;
	using TFloat64 = BINON_FLOAT64;
	
	static_assert(sizeof(TFloat32) == 4,
		"#define BINON_FLOAT32 to be a 32-bit floating-point type"
	);
	static_assert(sizeof(TFloat64) == 8,
		"#define BINON_FLOAT64 to be a 64-bit floating-point type"
	);
}

#endif
