#include "binon/binon.hpp"

#include <iostream>
#include <sstream>

static void DumpBinON(const std::string& s) {
	using std::cout;
	cout << "encoded value in hex:";
	std::size_t i = 0;
	for(auto c: s) {
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
}

auto main() -> int {
	try {
		using namespace binon;
		using namespace binon::types;
		using namespace std;

		SDict sd{kStrObjCode, kUIntCode};
		sd.value<StrObj,UInt>("foo") = 0;
		sd.value<StrObj,UInt>("bar") = 1;
		sd.value<StrObj,UInt>("baz") = 2;
		cout << "before encoding: " << sd << '\n';

		std::ostringstream oss;
		sd.encode(oss);

		DumpBinON(oss.str());

		std::istringstream iss{oss.str()};
		sd.decode(iss);
		cout << "after decoding: " << sd << '\n';
	}
	catch(const std::exception& err) {
		std::cerr << "ERROR: " << err.what() << '\n';
		return 1;
	}
	return 0;
}
