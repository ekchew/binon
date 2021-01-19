#ifndef BINON_IOUTIL_HPP
#define BINON_IOUTIL_HPP

#include "generator.hpp"
#include "macros.hpp"

#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace binon {

	using TStreamByte = BINON_STREAM_BYTE;
	using TStreamTraits = std::char_traits<TStreamByte>;
	using TIOS = std::basic_ios<TStreamByte,TStreamTraits>;
	using TIStream = std::basic_istream<TStreamByte,TStreamTraits>;
	using TOStream = std::basic_ostream<TStreamByte,TStreamTraits>;
	using TString = std::basic_string<
		TStreamByte, TStreamTraits, BINON_ALLOCATOR<TStreamByte>
		>;
	using TStringView = std::basic_string_view<TStreamByte,TStreamTraits>;
	static_assert(
		sizeof(TStreamByte) == 1,
		"BinON streams must use a 1-byte (binary) character type"
	);

	//	RequireIO is a simple class which sets a stream's exception flags in
	//	its constructor and restores them back to what they were in its
	//	destructor. This means any I/O errors should throw std::failure.
	struct RequireIO {

		//	The second "enable" argument defaults to true. If you make it
		//	false, RequireIO will effectively do nothing. This can be useful
		//	when you write a function that takes a requireIO argument. You can
		//	go through the motions of constructing a RequireIO object without
		//	necessarily setting the exception bits.
		RequireIO(TIOS& stream, bool enable=true);

		~RequireIO();

	private:
		TIOS* mPStream;
		TIOS::iostate mEx0;
	};

	//	Named constant to set second arg of RequireIO constructor false.
	constexpr bool kSkipRequireIO = false;
	
	template<typename ByteGen>
		auto StreamBytes(ByteGen& byteGen, TOStream& stream,
			bool requireIO=true) -> std::size_t
		{
			RequireIO rio{stream, requireIO};
			std::size_t count = 0u;
			for(auto byt: byteGen) {
				static_assert(sizeof byt == 1u,
					"binon::StreamBytes generator not yielding bytes");
				stream.write(reinterpret_cast<const TStreamByte*>(&byt), 1u);
				++count;
			}
			return count;
		}
	inline auto StreamedBytesGen(TIStream& stream, std::size_t count,
		bool requireIO=true)
		{
			auto nextByte = [&stream](decltype(count)& n) {
				TStreamByte byt;
				stream.read(&byt, 1u);
				--n;
				return reinterpret_cast<std::byte&>(byt);
			};
			auto nextOptByte = [count, nextByte](RequireIO&) mutable {
				return MakeOpt<std::byte>(count > 0u, nextByte, count);
			};
			return MakeGen<std::byte, RequireIO>(
				nextOptByte, stream, requireIO);
		}
}

#endif
