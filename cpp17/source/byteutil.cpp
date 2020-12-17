#include "binon/byteutil.hpp"
#include "binon/literals.hpp"

#include <algorithm>
#if !BINON_CPP20
	#include <cstring>
#endif

namespace binon {
	
	auto AsHexC(std::byte value, bool capitalize) noexcept -> std::array<char,3>
	{
		std::array<char,3> buffer;
		auto format = capitalize ? "%02X" : "%02x";
		std::sprintf(buffer.data(), format, value);
		return buffer;
	}
	auto AsHex(std::byte value, bool capitalize) -> std::string {
		return AsHexC(value, capitalize).data();
	}
	
#if !BINON_CPP20
	auto LittleEndian() noexcept -> bool {
		static const bool kLittleEndian = []{
			int i = 0xff;
			std::byte buf[sizeof i];
			std::memcpy(buf, &i, sizeof buf);
			return buf[0] == 0xff_byte;
		}();
		return kLittleEndian;
	}
#endif

}
