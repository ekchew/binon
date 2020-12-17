#include "binon/binon.hpp"

#include <iostream>
#include <sstream>

auto main() -> int {
	try {
		using std::cout;
		
		binon::UInt i{42};
		cout << "i before encoding: " << i << '\n';
		
		std::ostringstream oss;
		i.encode(oss);
		cout << "encoded i: ";
		for(auto c: oss.str()) {
			cout << binon::AsHex(binon::ToByte(c));
		}
		cout << '\n';
		
		std::istringstream iss{oss.str()};
		auto pObj = binon::BinONObj::Decode(iss);
		cout << "i after decoding: " << pObj->getUInt64() << '\n';
	}
	catch(std::exception& err) {
		std::cerr << "ERROR: " << err.what() << '\n';
		return 1;
	}
	return 0;
}
