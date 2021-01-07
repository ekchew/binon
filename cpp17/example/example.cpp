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
	using namespace binon;
	using namespace binon::types;
	using namespace std;
	try {

		SDict sd{kStrObjCode, kUIntCode};
		sd.value<StrObj,UInt>("foo") = 0;
		sd.value<StrObj,UInt>("bar") = 1;
		sd.value<StrObj,UInt>("baz") = 2;
		cout << "before encoding: " << sd << '\n';

		ostringstream oss;
		sd.encode(oss);
		auto s{move(oss).str()};

		DumpBinON(s);

		istringstream iss{move(s)};
		sd.decode(iss);
		cout << "after decoding: " << sd << '\n';
		
		SListT<IntObj> slt{0, 1, 2};
		cout << "before encoding: " << slt << '\n';

		oss = ostringstream{};
		slt.encode(oss);
		s = move(oss).str();

		DumpBinON(s);

		iss = istringstream{move(s)};
		slt.decode(iss);
		cout << "after decoding: " << slt << '\n';
	}
	catch(const exception& err) {
		cerr << "ERROR: " << err.what() << '\n';
		return 1;
	}
	return 0;
}
