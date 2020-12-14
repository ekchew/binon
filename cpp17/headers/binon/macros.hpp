#ifndef BINON_MACROS_HPP
#define BINON_MACROS_HPP

static_assert(__cplusplus > 201402L, "BinON requires C++17 or later");

//	If, for some reason, you want BinON to use a custom allocator for all
//	it's internal memory allocations, you can set this precompiler option.
#ifndef BINON_ALLOCATOR
	#define BINON_ALLOCATOR std::allocator
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
	#pragma message "C++20 detected."
	#define BINON_CPP20 true
	#define BINON_CPP20_CONSTEXPR constexpr
#else
	#define BINON_CPP20 false
	#define BINON_CPP20_CONSTEXPR
#endif

//	Some purported C++17 compilers do not seem to support execution policies,
//	so here we check if the <execution> header is available.
#if defined(__has_include) && __has_include(<execution>)
	#include <execution>
	#define BINON_PAR_UNSEQ std::execution::par_unseq,
#else
	#pragma message "C++17 execution policies unavailable."
	#define BINON_PAR_UNSEQ
#endif

#endif
