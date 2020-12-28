#include "binon/binon.hpp"

#include <iostream>
#include <sstream>

auto main() -> int {
	try {
		using std::cout;
		
		binon::IntObj value{42};
		cout << "before encoding: " << value << '\n';
		
		std::ostringstream oss;
		value.encode(oss);
		cout << "encoded value in hex:";
		std::size_t i = 0;
		for(auto c: oss.str()) {
			if((i & 0xf) == 0x0) {
				cout << "\n\t";
			}
			else if((i & 0x3) == 0x0) {
				cout << ' ';
			}
			cout << binon::AsHex(binon::ToByte(c));
			++i;
		}
		cout << '\n';
		
		std::istringstream iss{oss.str()};
		value = binon::BinONObj::DecodeType<decltype(value)>(iss);
		cout << "after decoding: " << value << '\n';
	}
	catch(std::exception& err) {
		std::cerr << "ERROR: " << err.what() << '\n';
		return 1;
	}
	return 0;
}
