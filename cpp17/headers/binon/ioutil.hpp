#ifndef BINON_IOUTIL_HPP
#define BINON_IOUTIL_HPP

#include <istream>
#include <ostream>

namespace binon {

	//	BinON I/O is currently hard-wired to the default char-based iostreams.
	//	In theory, you could change BINON_STREAM_BYTE to a different data type
	//	but it would need to be byte-length for BinON to work, so it's
	//	probably best to leave these type definitions alone.
	#ifndef BINON_STREAM_BYTE
		#define BINON_STREAM_BYTE std::ios::char_type
	#endif
	using StreamByte = BINON_STREAM_BYTE;
	using StreamTraits = std::char_traits<StreamByte>;
	using IOS = std::basic_ios<StreamByte,StreamTraits>;
	using IStream = std::basic_istream<StreamByte,StreamTraits>;
	using OStream = std::basic_ostream<StreamByte,StreamTraits>;
	static_assert(
		sizeof(StreamByte) == 1,
		"BinON streams must use a 1-byte (binary) character type"
	);
	
	//	RequireIO is a simple class which sets a stream's exception flags in
	//	its constructor and restores them back to what they were in its
	//	destructor. This means any I/O errors should throw std::failure.
	class RequireIO {
	public:
		
		//	The second "enable" argument defaults to true. If you make it
		//	false, RequireIO will effectively do nothing. This can be useful
		//	when you write a function that takes a requireIO argument. You can
		//	go through the motions of constructing a RequireIO object without
		//	necessarily setting the exception bits.
		RequireIO(IOS& stream, bool enable=true);
		
		~RequireIO();
	
	private:
		IOS* mPStream;
		IOS::iostate mEx0;
	};

}

#endif
