#ifndef BINON_MACROS_HPP
#define BINON_MACROS_HPP

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
	#define BINON_CPP20 true
	#define BINON_CPP20_CONSTEXPR constexpr
#else
	#define BINON_CPP20 false
	#define BINON_CPP20_CONSTEXPR
#endif

#endif
