#include "binon/binon.hpp"

#include <cmath>
#include <iostream>
#include <sstream>

void DumpBinON(const std::string& s) {
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
template<typename T>
	void RunTest(T& v) {
		using namespace std;
		cout << "before encoding:\n\t" << v << '\n';
		ostringstream oss;
		v.encode(oss);
		auto s{std::move(oss).str()};
		DumpBinON(s);
		istringstream iss{std::move(s)};
		v.decode(iss);
		cout << "after decoding:\n\t" << v << '\n';
	}

auto main() -> int {
	using namespace binon;
	using namespace binon::types;
	try {
		SDictT<std::string, int> v1;
		v1.mValue["foo"] = 0;
		v1.mValue["bar"] = 1;
		v1.mValue["baz"] = 2;
		RunTest(v1);
		SKDictT<StrObj> v2;
		v2.mValue["foo"] = std::make_shared<IntObj>(42);
		v2.mValue["bar"] = std::make_shared<StrObj>("qux");
		v2.mValue["baz"] = std::make_shared<BoolObj>(true);
		RunTest(v2);
		SListT<bool> v3{true,true,false,true,true,true,false};
		RunTest(v3);
	}
	catch(const std::exception& err) {
		std::cerr << "ERROR: " << err.what() << '\n';
		return 1;
	}
	return 0;
}
