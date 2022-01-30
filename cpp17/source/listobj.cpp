#include "binon/listobj.hpp"
#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"
#include "binon/packelems.hpp"
#include "binon/binonobj.hpp"

#include <sstream>
#if BINON_LIB_EXECUTION
	#include <execution>
	#define BINON_PAR_UNSEQ std::execution::par_unseq,
#else
	#if BINON_EXEC_POLICIES
		#pragma message "C++17 execution policies unavailable"
	#endif
	#define BINON_PAR_UNSEQ
#endif

namespace binon {

	//---- ListObj ------------------------------------------------------------

	auto ListObj::encodeData(TOStream& stream, bool requireIO) const
		-> const ListObj&
	{
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		for(auto& v: u) {
			v.encode(stream, kSkipRequireIO);
		}
		return *this;
	}
	auto ListObj::decodeData(TIStream& stream, bool requireIO)
		-> ListObj&
	{
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		u.resize(0);
		u.reserve(n);
		while(n-->0) {
			u.push_back(BinONObj::Decode(stream, kSkipRequireIO));
		}
		return *this;
	}
	void ListObj::printArgs(std::ostream& stream) const {
		stream << "ListObj::TValue{";
		auto& u = value();
		bool first = true;
		for(auto& v: u) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			v.print(stream);
		}
		stream << "}";
	}

	//---- SList --------------------------------------------------------------

	SList::SList(std::any value, CodeByte elemCode):
		StdCtnr<SList,TValue>{std::move(value)},
		mElemCode{elemCode}
	{
	}
	SList::SList(CodeByte elemCode):
		mElemCode{elemCode}
	{
	}
	auto SList::encodeData(TOStream& stream, bool requireIO) const
		-> const SList&
	{
		if(mElemCode == kNoObjCode) {
			std::ostringstream oss;
			oss << "SList is missing an element code (";
			BinONObj{*this}.print(oss);
			oss << ')';
			throw TypeErr{oss.str()};
		}
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		mElemCode.write(stream, kSkipRequireIO);
		PackElems pack{mElemCode, stream};
		for(auto& v: u) {
			pack(v, kSkipRequireIO);
		}
		return *this;
	}
	auto SList::decodeData(TIStream& stream, bool requireIO)
		-> SList&
	{
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.mValue.scalar();
		mElemCode = CodeByte::Read(stream, kSkipRequireIO);
		u.resize(0);
		UnpackElems unpack{mElemCode, stream};
		while(n-->0) {
			u.push_back(unpack(kSkipRequireIO));
		}
		return *this;
	}
	void SList::printArgs(std::ostream& stream) const {
		stream << "SList::TValue{";
		auto& u = value();
		bool first = true;
		for(auto& v: u) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			v.print(stream);
		}
		stream << "}, ";
		mElemCode.printRepr(stream);
	}

}
