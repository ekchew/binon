#include "binon/byteutil.hpp"

#include <algorithm>
#if __cplusplus <= 201703L
	#include <cstring>
#endif

namespace binon {
	
	auto AsHex(std::byte value, bool capitalize) -> std::array<char,3>
	{
		std::array<char,3> buffer;
		auto format = capitalize ? "%02X" : "%02x";
		std::sprintf(buffer.data(), format, value);
		return buffer;
	}
	
#if __cplusplus <= 201703L
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
