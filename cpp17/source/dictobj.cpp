#include "binon/dictobj.hpp"
#include "binon/listobj.hpp"
#include "binon/packelems.hpp"
#include "binon/binonobj.hpp"

#include <iostream>

namespace binon {

	//---- DictBase ------------------------------------------------------------

	auto DictBase::operator == (const DictBase& rhs) const -> bool {
		auto& da = value();
		auto& db = rhs.value();
		if(da.size() != db.size()) {
			return false;
		}
		for(auto& a: da) {
			auto pb = db.find(a.first);
			if(pb == db.end() || a.second != pb->second) {
				return false;
			}
		}
		return true;
	}
	auto DictBase::operator != (const DictBase& rhs) const -> bool {
		return !(*this == rhs);
	}
	auto DictBase::hasDefVal() const -> bool {
		return value().size() == 0;
	}
	auto DictBase::value() & -> TValue& {
		if(!mValue.has_value()) {
			mValue = TValue();
		}
		try {
			return std::any_cast<TValue&>(mValue);
		}
		catch(std::bad_any_cast&) {
			CastError();
		}

	}
	auto DictBase::value() && -> TValue {
		if(!mValue.has_value()) {
			mValue = TValue();
		}
		try {
			return std::any_cast<TValue&&>(std::move(mValue));
		}
		catch(std::bad_any_cast&) {
			CastError();
		}
	}
	auto DictBase::value() const& -> const TValue& {
		return const_cast<DictBase*>(this)->value();
	}
	auto DictBase::size() const -> std::size_t {
		return value().size();
	}
	auto DictBase::calcHash(std::size_t seed) const -> std::size_t {
		for(auto& pair: value()) {
			seed ^= Hash(pair.first, pair.second);
		}
		return seed;
	}

	//---- DictObj -------------------------------------------------------------

	auto DictObj::encodeData(TOStream& stream, bool requireIO) const
		-> const DictObj&
	{
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		for(auto& [k, v]: u) {
			k.encode(stream, kSkipRequireIO);
		}
		for(auto& [k, v]: u) {
			v.encode(stream, kSkipRequireIO);
		}
		return *this;
	}
	auto DictObj::decodeData(TIStream& stream, bool requireIO)
		-> DictObj&
	{
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		std::vector<BinONObj> ks;
		ks.reserve(n);
		u.clear();
		u.reserve(n);
		while(n-->0) {
			ks.push_back(BinONObj::Decode(stream, kSkipRequireIO));
		}
		for(auto& k: ks) {
			u[std::move(k)] = BinONObj::Decode(stream, kSkipRequireIO);
		}
		return *this;
	}
	auto DictObj::hash() const -> std::size_t {
		return calcHash(std::hash<std::string_view>{}(kClsName));
	}
	void DictObj::printArgs(std::ostream& stream) const {
		stream << "DictObj::TValue{";
		auto& u = value();
		bool first = true;
		for(auto& [k, v]: u) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			stream << '{';
			k.print(stream);
			stream << ", ";
			v.print(stream);
			stream << '}';
		}
		stream << "}";
	}

	//---- SKDict --------------------------------------------------------------

	SKDict::SKDict(const std::any& value, CodeByte keyCode):
		DictBase{value},
		mKeyCode{keyCode}
	{
	}
	SKDict::SKDict(std::any&& value, CodeByte keyCode) noexcept:
		DictBase{std::move(value)},
		mKeyCode{keyCode}
	{
	}
	SKDict::SKDict(CodeByte keyCode) noexcept:
		mKeyCode{keyCode}
	{
	}
	auto SKDict::encodeData(TOStream& stream, bool requireIO) const
		-> const SKDict&
	{
		if(mKeyCode == kNoObjCode) {
			std::ostringstream oss;
			oss << "SKDict is missing a key code (";
			BinONObj{*this}.print(oss);
			oss << ')';
			throw TypeErr{oss.str()};
		}
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		mKeyCode.write(stream, kSkipRequireIO);
		{
			PackElems packKey{mKeyCode, stream};
			for(auto& [k, v]: u) {
				packKey(k, kSkipRequireIO);
			}
		}
		for(auto& [k, v]: u) {
			v.encode(stream, kSkipRequireIO);
		}
		return *this;
	}
	auto SKDict::decodeData(TIStream& stream, bool requireIO)
		-> SKDict&
	{
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		mKeyCode = CodeByte::Read(stream, kSkipRequireIO);
		std::vector<BinONObj> ks;
		ks.reserve(n);
		u.clear();
		u.reserve(n);
		UnpackElems unpackKey{mKeyCode, stream};
		while(n-->0) {
			ks.push_back(unpackKey(kSkipRequireIO));
		}
		for(auto& k: ks) {
			u[std::move(k)] = BinONObj::Decode(stream, kSkipRequireIO);
		}
		return *this;
	}
	auto SKDict::hash() const -> std::size_t {
		return calcHash(std::hash<std::string_view>{}(kClsName));
	}
	void SKDict::printArgs(std::ostream& stream) const {
		stream << "SKDict::TValue{";
		auto& u = value();
		bool first = true;
		for(auto& [k, v]: u) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			stream << '{';
			k.print(stream);
			stream << ", ";
			v.print(stream);
			stream << '}';
		}
		stream << "}, ";
		mKeyCode.printRepr(stream);
	}

	//---- SDict ---------------------------------------------------------------

	SDict::SDict(const std::any& value, CodeByte keyCode, CodeByte valCode):
		DictBase{value},
		mKeyCode{keyCode},
		mValCode{valCode}
	{
	}
	SDict::SDict(std::any&& value, CodeByte keyCode, CodeByte valCode) noexcept:
		DictBase{std::move(value)},
		mKeyCode{keyCode},
		mValCode{valCode}
	{
	}
	SDict::SDict(CodeByte keyCode, CodeByte valCode) noexcept:
		mKeyCode{keyCode},
		mValCode{valCode}
	{
	}
	auto SDict::encodeData(TOStream& stream, bool requireIO) const
		-> const SDict&
	{
		if(mKeyCode == kNoObjCode || mValCode == kNoObjCode) {
			std::ostringstream oss;
			oss << "SDict is missing a";
			if(mKeyCode == kNoObjCode) {
				oss << " key";
				if(mValCode == kNoObjCode) {
					oss << " and value";
				}
			}
			else {
				oss << " value";
			}
			oss << " code (";
			BinONObj{*this}.print(oss);
			oss << ')';
			throw TypeErr{oss.str()};
		}
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		mKeyCode.write(stream, kSkipRequireIO);
		{
			PackElems packKey{mKeyCode, stream};
			for(auto& [k, v]: u) {
				packKey(k, kSkipRequireIO);
			}
		}
		mValCode.write(stream, kSkipRequireIO);
		{
			PackElems packVal{mValCode, stream};
			for(auto& [k, v]: u) {
				packVal(v, kSkipRequireIO);
			}
		}
		return *this;
	}
	auto SDict::decodeData(TIStream& stream, bool requireIO)
		-> SDict&
	{
		RequireIO rio{stream, requireIO};
		auto& u = value();
		UIntObj sizeObj;
		sizeObj.decodeData(stream, kSkipRequireIO);
		auto n = sizeObj.value().scalar();
		mKeyCode = CodeByte::Read(stream, kSkipRequireIO);
		std::vector<BinONObj> ks;
		ks.reserve(n);
		u.clear();
		u.reserve(n);
		UnpackElems unpackKey{mKeyCode, stream};
		while(n-->0) {
			ks.push_back(unpackKey(kSkipRequireIO));
		}
		mValCode = CodeByte::Read(stream, kSkipRequireIO);
		UnpackElems unpackVal{mValCode, stream};
		for(auto& k: ks) {
			u[std::move(k)] = unpackVal(kSkipRequireIO);
		}
		return *this;
	}
	auto SDict::hash() const -> std::size_t {
		return calcHash(std::hash<std::string_view>{}(kClsName));
	}
	void SDict::printArgs(std::ostream& stream) const {
		stream << "SDict::TValue{";
		auto& u = value();
		bool first = true;
		for(auto& [k, v]: u) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			stream << '{';
			k.print(stream);
			stream << ", ";
			v.print(stream);
			stream << '}';
		}
		stream << "}, ";
		mKeyCode.printRepr(stream);
		stream << ", ";
		mValCode.printRepr(stream);
	}
}
