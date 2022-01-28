#include "binon/listobj.hpp"
#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"
#include "binon/packelems.hpp"
#include "binon/varobj.hpp"

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

	//---- TListObj ------------------------------------------------------------

	auto TListObj::encodeData(TOStream& stream, bool requireIO) const
		-> const TListObj&
	{
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		for(auto& v: u) {
			v.encode(stream, kSkipRequireIO);
		}
		return *this;
	}
	auto TListObj::decodeData(TIStream& stream, bool requireIO)
		-> TListObj&
	{
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		u.resize(0);
		u.reserve(n);
		while(n-->0) {
			u.push_back(VarObj::Decode(stream, kSkipRequireIO));
		}
		return *this;
	}
	void TListObj::printArgs(std::ostream& stream) const {
		stream << "TListObj::TValue{";
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

	//---- TSList --------------------------------------------------------------

	TSList::TSList(std::any value, CodeByte elemCode):
		StdCtnr<TSList,TValue>{std::move(value)},
		mElemCode{elemCode}
	{
	}
	TSList::TSList(CodeByte elemCode):
		mElemCode{elemCode}
	{
	}
	auto TSList::encodeData(TOStream& stream, bool requireIO) const
		-> const TSList&
	{
		if(mElemCode == kNoObjCode) {
			std::ostringstream oss;
			oss << "TSList is missing an element code (";
			VarObj{*this}.print(oss);
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
	auto TSList::decodeData(TIStream& stream, bool requireIO)
		-> TSList&
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
	void TSList::printArgs(std::ostream& stream) const {
		stream << "TSList::TValue{";
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
