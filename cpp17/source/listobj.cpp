#include "binon/listobj.hpp"
#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"
#include "binon/macros.hpp"
#include "binon/packelems.hpp"
#include "binon/binonobj.hpp"

#include <algorithm>
#include <sstream>

namespace binon {

	//---- CtnrBase ------------------------------------------------------------

	CtnrBase::CtnrBase(const std::any& ctnr):
		mValue{ctnr}
	{
	}
	CtnrBase::CtnrBase(std::any&& ctnr) noexcept:
		mValue{std::move(ctnr)}
	{
	}
	[[noreturn]] void CtnrBase::CastError() {
		throw TypeErr{
			"BinON container object constructed with unexpected value type"
		};
	}

	//---- ListBase ------------------------------------------------------------

	auto ListBase::operator == (const ListBase& rhs) const -> bool {
		auto& a = value();
		auto& b = rhs.value();
		return std::equal(BINON_PAR
			a.begin(), a.end(), b.begin(), b.end()
		);
	}
	auto ListBase::operator != (const ListBase& rhs) const -> bool {
		return !(*this == rhs);
	}
	auto ListBase::hasDefVal() const -> bool {
		return value().size() == 0;
	}
	auto ListBase::value() & -> TList& {
		if(!mValue.has_value()) {
			mValue = TList();
		}
		try {
			return std::any_cast<TList&>(mValue);
		}
		catch(std::bad_any_cast&) {
			CastError();
		}

	}
	auto ListBase::value() && -> TList {
		if(!mValue.has_value()) {
			mValue = TValue();
		}
		try {
			return std::any_cast<TList&&>(std::move(mValue));
		}
		catch(std::bad_any_cast&) {
			CastError();
		}
	}
	auto ListBase::value() const& -> const TList& {
		return const_cast<ListBase*>(this)->value();
	}
	auto ListBase::size() const -> std::size_t {
		return value().size();
	}
	auto ListBase::valueHash(std::size_t seed) const -> std::size_t {
		for(auto& elem: value()) {
			seed = HashCombine(seed, std::hash<BinONObj>{}(elem));
		}
		return seed;
	}

	//---- ListObj -------------------------------------------------------------

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
	auto ListObj::hash() const -> std::size_t {
		return valueHash(std::hash<std::string_view>{}(kClsName));
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
		ListBase::ListBase{std::move(value)},
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
	auto SList::hash() const -> std::size_t {
		return valueHash(std::hash<std::string_view>{}(kClsName));
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
