#include "binon/packelems.hpp"

#include <algorithm>
#include <sstream>

namespace binon {

	//---- CtnrBase ------------------------------------------------------------

	CtnrBase::CtnrBase(const CtnrBase& ctnrBase):
		mValue{ctnrBase.mValue}
	{
	}
	CtnrBase::CtnrBase(CtnrBase&& ctnrBase) noexcept:
		mValue{std::move(ctnrBase.mValue)}
	{
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
	auto ListBase::value() & -> TValue& {
		if(!mValue.has_value()) {
			mValue = TValue();
		}
		try {
			return std::any_cast<TValue&>(mValue);
		}
		catch(std::bad_any_cast&) {
			this->castError<TValue>();
		}

	}
	auto ListBase::value() && -> TValue {
		if(!mValue.has_value()) {
			mValue = TValue();
		}
		try {
			return std::any_cast<TValue&&>(std::move(mValue));
		}
		catch(std::bad_any_cast&) {
			this->castError<TValue>();
		}
	}
	auto ListBase::value() const& -> const TValue& {
		return const_cast<ListBase*>(this)->value();
	}
	auto ListBase::size() const -> std::size_t {
		return value().size();
	}
	auto ListBase::calcHash(std::size_t seed) const -> std::size_t {
		for(auto& elem: value()) {
			seed = HashCombine(seed, std::hash<BinONObj>{}(elem));
		}
		return seed;
	}

	//---- ListObj -------------------------------------------------------------

	ListObj::ListObj(const SList& obj) {
		this->mValue = obj.value();
	}
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
		return calcHash(std::hash<CodeByte>{}(kTypeCode));
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
		mElemCode{elemCode}
	{
		this->mValue = std::move(value);
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
			throw NoTypeCode{oss.str()};
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
		return calcHash(std::hash<CodeByte>{}(kTypeCode));
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
