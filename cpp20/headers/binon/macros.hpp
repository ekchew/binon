#ifndef BINON_MACROS_HPP
#define BINON_MACROS_HPP

static_assert(__cplusplus > 201703L, "BinON requires C++20 or later");

#include <concepts>
#include <version>

//	By default, BINON_DEBUG is set true if a DEBUG macro is defined or
//	false otherwise.
#ifndef BINON_DEBUG
	#if defined(DEBUG) && DEBUG
		#define BINON_DEBUG true
	#else
		#define BINON_DEBUG false
	#endif
#endif
#if BINON_DEBUG
	#define BINON_IF_DEBUG(code) code
	#define BINON_IF_RELEASE(code)
	#define BINON_IF_DBG_REL(dbg,rel) dbg
#else
	#define BINON_IF_DEBUG(code)
	#define BINON_IF_RELEASE(code) code
	#define BINON_IF_DBG_REL(dbg,rel) rel
#endif

//	If, for some reason, you want BinON to use a custom allocator for all
//	it's internal memory allocations, you can set this precompiler option.
#ifndef BINON_ALLOCATOR
	#define BINON_ALLOCATOR std::allocator
#endif

//	binon can make mild use of execution policies to auto-parallelize certain
//	operations. Given what a pain library support is for these, however, they
//	are not used by default. Define BINON_EXEC_POLICIES true if you want them.
#ifndef BINON_EXEC_POLICIES
	#define BINON_EXEC_POLICIES false
#endif
#if BINON_EXEC_POLICIES && defined(__cpp_lib_execution)
	#include <execution>
	#define BINON_LIB_EXECUTION true
	#define BINON_PAR std::execution::par,
	#define BINON_PAR_UNSEQ std::execution::par_unseq,
	#define BINON_UNSEQ std::execution::unseq,
#else
	#define BINON_LIB_EXECUTION false
	#define BINON_PAR
	#define BINON_PAR_UNSEQ
	#define BINON_UNSEQ
#endif

//	Though std::bit_cast is a core C++20 feature, as of this writing, the Apple
//	version of clang does not support it.
#if BINON_CPP20 && defined(__cpp_lib_bit_cast)
    #define BINON_BIT_CAST true
	#define BINON_BIT_CAST_CONSTEXPR constexpr
#else
    #define BINON_BIT_CAST false
	#define BINON_BIT_CAST_CONSTEXPR
#endif

//	BinON I/O is currently hard-wired to the default char-based iostreams.
//	In theory, you could change BINON_STREAM_BYTE to a different data type
//	but it would need to be byte-length for BinON to work, so it's probably
//	best to leave it alone.
#ifndef BINON_STREAM_BYTE
	#define BINON_STREAM_BYTE std::ios::char_type
#endif

//	Comma escape for macros that take arguments.
#define BINON_COMMA ,

//	BINON_RESTRICT evaluates to __restrict where available.
#if defined(__GNUC__) && __GNUC__ > 3
	#define BINON_RESTRICT __restrict
#elif defined(_MSC_VER) && _MSC_VER >= 1400
	#define BINON_RESTRICT __restrict
#else
	#define BINON_RESTRICT
#endif

//	This build requires C++20 or later and concept support, so the below are
//	not necessary for the BinON library itself anymore but left in for any
//	external code that still relies on them.
#define BINON_CPP20 true
#define BINON_IF_CPP20(code) code
#define BINON_IF_CPP20_ELSE(code, alt) code
#define BINON_CONCEPTS true
#define BINON_IF_CONCEPTS(code) code
#define BINON_NO_CONCEPTS(code)
#define BINON_IF_CONCEPTS_ELSE(code, alt) code
#define BINON_CONCEPT(C) C
#define BINON_AUTO(C) C auto
#define BINON_CONCEPTS_FN(req, cond, res) -> res requires req
#define BINON_CONCEPTS_CONSTRUCTOR(req, cond, ext) ) ext requires req
#define BINON_CONCEPTS_CONSTRUCTOR_DEF(req, cond, ext) ) ext requires req

#endif
