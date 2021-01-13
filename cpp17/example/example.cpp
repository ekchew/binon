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

template<typename T=int, typename Fn=void> struct Foo {
	Fn fn;
	Foo(Fn fn): fn{fn} {}
	T call(T val) { return fn(val); }
};
Foo foo{[](int val) { return val; }};

auto main() -> int {
	std::cout << foo.call(42) << '\n';
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

		SListT<bool> slt{true, false, true};
		cout << "before encoding: " << slt << '\n';

		oss = ostringstream{};
		slt.encode(oss);
		s = move(oss).str();

		DumpBinON(s);

		iss = istringstream{move(s)};
		slt.decode(iss);
		cout << "after decoding: " << slt << '\n';
		
		/*
		std::vector<bool> bools = {
			true, true, false, true, true, true, false, false,
			true, true, true, false, true, true, true
		};
		std::size_t boolCount;
		for(auto byt: PackedBoolsGen(bools.begin(), bools.end(), boolCount)) {
			std::cout << AsHex(byt);
		}
		std::cout << "\nbool count: " << boolCount << '\n';
		Foo([](int i) {});
		*/
		/*
		auto gen = MakeGenerator(
			[i = 0]() mutable {
				return ++i, MakeOpt(i <= 5, i);
			}
		);
		for(auto i: gen) {
			std::cout << i << '\n';
		}
		*/
		/*
		auto gen = MakeGenerator<int>(
			[i = 0](int& j) mutable {
				return ++i, MakeOpt(i <= 5, i + j);
			}, 10
		);
		for(auto i: gen) {
			std::cout << i << '\n';
		}
		*/
		auto gen = MakeGenerator<int>(
			[](int& i) {
				return ++i, MakeOpt(i <= 5, i);
			}, 0);
		for(auto i: gen) {
			std::cout << i << '\n';
		}
		std::cout << "final gen.mData: " << gen.mData << '\n';
	}
	catch(const exception& err) {
		cerr << "ERROR: " << err.what() << '\n';
		return 1;
	}
	return 0;
}
