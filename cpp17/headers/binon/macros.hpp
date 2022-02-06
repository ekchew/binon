#ifndef BINON_MACROS_HPP
#define BINON_MACROS_HPP

static_assert(__cplusplus > 201402L, "BinON requires C++17 or later");

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

//	BinON I/O is currently hard-wired to the default char-based iostreams.
//	In theory, you could change BINON_STREAM_BYTE to a different data type
//	but it would need to be byte-length for BinON to work, so it's probably
//	best to leave it alone.
#ifndef BINON_STREAM_BYTE
	#define BINON_STREAM_BYTE std::ios::char_type
#endif

//	Macros that kick in if C++20 or later is available.
#if __cplusplus > 201703L
	#define BINON_CPP20 true
	#define BINON_IF_CPP20(code) code
#else
	#define BINON_CPP20 false
	#define BINON_IF_CPP20(code)
#endif

//	Here we look for specific C++ features.
#ifdef __has_include

	//	Later versions of C++ are supposed to supply a <version> header
	//	containing all the language feature macros. If unavailable, we will need
	//	to load specific headers to check for the relevant macros (e.g.
	//	<memory> for memory-related macros).
	#if __has_include(<version>)
		#include <version>
		#define BINON_GOT_VERSION true
	#else
		#define BINON_GOT_VERSION false
	#endif

	//	Some purported C++17 compilers (e.g. clang++) do not seem to support
	//	execution policies, so here we check if they are really available.
	//	(Note that you may want to go link the tbb library to get some proper
	//	parallelism going.)
	#if BINON_EXEC_POLICIES && __has_include(<execution>)
		#if !BINON_GOT_VERSION
			#include <execution>
		#endif
		#ifdef __cpp_lib_execution
			#define BINON_LIB_EXECUTION true
		#endif
	#endif

	//	See if C++20 concepts are available.
	#if BINON_CPP20 && __has_include(<concepts>)
		#if !BINON_GOT_VERSION
			#include <concepts>
		#endif
		#ifdef __cpp_concepts
			#define BINON_CONCEPTS true
			#define BINON_IF_CONCEPTS(code) code
			#if BINON_GOT_VERSION
				#include <concepts>
			#endif
		#endif
	#endif
#else
	#pragma message "__has_include macro unavailable"
#endif

//	Set defaults for when the above language features are missing.
#ifndef BINON_LIB_EXECUTION
	#define BINON_LIB_EXECUTION false
#endif
#ifndef BINON_CONCEPTS
	#define BINON_CONCEPTS false
#endif
#if !BINON_CONCEPTS
	#define BINON_IF_CONCEPTS(code)
#endif

//	Defines to help use execution policies where available
#if BINON_LIB_EXECUTION
	#include <execution>
	#define BINON_PAR std::execution::par,
	#define BINON_PAR_UNSEQ std::execution::par_unseq,
	#define BINON_UNSEQ std::execution::unseq,
#else
	#if BINON_EXEC_POLICIES
		#pragma message "C++17 execution policies unavailable"
	#endif
	#define BINON_PAR
	#define BINON_PAR_UNSEQ
	#define BINON_UNSEQ
#endif

//	Comma escape for macros that take arguments.
#define BINON_COMMA ,

#endif
