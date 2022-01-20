#include "binon/listobj.hpp"
#include "binon/boolobj.hpp"
#include "binon/intobj.hpp"
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

	//---- TListBase -----------------------------------------------------------

	TListBase::TListBase(const TValue& value):
		mValue{value}
	{
	}
	TListBase::TListBase(TValue&& value):
		mValue{std::forward<TValue>(value)}
	{
	}
	TListBase::TListBase():
		mValue{TValue()}
	{
	}
	auto TListBase::value() -> TValue& {
		return std::any_cast<TValue&>(mValue);
	}
	auto TListBase::value() const -> const TValue& {
		return std::any_cast<const TValue&>(mValue);
	}
	auto TListBase::hashValue(std::size_t seed) const -> std::size_t {
		auto& u = value();
		for(auto& v: u) {
			seed = HashCombine(seed, std::hash<TVarObj>{}(v));
		}
		return seed;
	}
	auto TListBase::sameValue(const TListBase& other) const -> bool {
		auto& u = value();
		auto& v = other.value();
		auto n = u.size();
		if(n != v.size()) {
			return false;
		}
		for(decltype(n) i = 0; i < n; ++i) {
			if(u[i] != v[i]) {
				return false;
			}
		}
		return true;
	}

	//---- TListObj ------------------------------------------------------------

	auto TListObj::hash() const -> std::size_t {
		return hashValue(std::hash<CodeByte>{}(kTypeCode));
	}
	void TListObj::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto& u = value();
		TUIntObj{u.size()}.encodeData(stream, kSkipRequireIO);
		for(auto& v: u) {
			v.encode(stream, kSkipRequireIO);
		}
	}
	void TListObj::decodeData(CodeByte cb, TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		auto& u = value();
		TUIntObj sizeObj;
		sizeObj.decodeData(sizeObj.kTypeCode, stream, kSkipRequireIO);
		auto n = sizeObj.mValue;
		u.resize(0);
		while(n-->0) {
			u.push_back(TVarObj::Decode(stream, kSkipRequireIO));
		}
	}

	//---- TSList --------------------------------------------------------------

	auto TSList::operator== (const TSList& rhs) const -> bool {
		return mElemCode == rhs.mElemCode && sameValue(rhs);
	}
	auto TSList::hash() const noexcept -> std::size_t {
		return hashValue(Hash(kTypeCode, mElemCode));
	}

	//--------------------------------------------------------------------------

	auto DeepCopyTList(const TList& list) -> TList {
		TList copy(list.size());
		std::transform(BINON_PAR_UNSEQ
			list.begin(), list.end(), copy.begin(),
			[](const TList::value_type& p)
				{ return p->makeCopy(kDeepCopy); }
			);
		return std::move(copy);
	}
	void PrintTListRepr(const TList& list, std::ostream& stream) {
		stream << "TList{";
		bool first = true;
		for(auto&& pElem: list) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			pElem->printPtrRepr(stream);
		}
		stream << '}';
	}

	//---- ListBase ------------------------------------------------------------

	auto ListBase::hasValue(TList::size_type i) const -> bool {
		auto& lst = list();
		return i < lst.size() && static_cast<bool>(lst[i]);
	}

	//---- ListObj -------------------------------------------------------------

	void ListObj::EncodeData(const TValue& v, TOStream& stream, bool requireIO)
	{
		RequireIO rio{stream, requireIO};
		UIntObj::EncodeData(v.size(), stream, kSkipRequireIO);
		EncodeElems(IterGen{v.begin(), v.end()}, stream, kSkipRequireIO);
	}
	auto ListObj::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		RequireIO rio{stream, requireIO};
		auto count = UIntObj::DecodeData(stream, kSkipRequireIO);
		TValue v(count);
		v.clear();
		for(auto&& elem: DecodedElemsGen(stream, count, requireIO)) {
			v.push_back(elem);
		}
		return std::move(v);
	}
	void ListObj::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	void ListObj::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}
	auto ListObj::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<ListObj>(DeepCopyTList(mValue));
		}
		return std::make_shared<ListObj>(*this);
	}

	//---- SList ---------------------------------------------------------------

	void SList::AssertElemTypes(const TValue& v) {
		auto n = v.mList.size();
		for(decltype(n) i = 0; i < n; ++i) {
			auto tc = v.mList[i]->typeCode();
			if(tc != v.mElemCode) {
				std::ostringstream oss;
				auto hex0 = AsHex(v.mElemCode);
				auto hex1 = AsHex(tc);
				oss	<< "SList element " << i << " type code 0x" << hex1
					<< " does not match expected 0x" << hex0
					<< " (element value: ";
				v.mList[i]->printRepr(oss);
				oss << ", list size: " << v.mList.size() << ")";
				throw TypeErr{oss.str()};
			}
		}
	}
	void SList::EncodeData(
		const TValue& v, TOStream& stream,
		bool requireIO, bool assertTypes)
	{
		RequireIO rio{stream, requireIO};
		UIntObj::EncodeData(v.mList.size(), stream, kSkipRequireIO);
		EncodeElems(v, stream, kSkipRequireIO);
	}
	void SList::EncodeElems(
		const TValue& v, TOStream& stream,
		bool requireIO, bool assertTypes)
	{
		//	Make sure all elements in the list are of the correct type?
		if(assertTypes) {
			AssertElemTypes(v);
		}

		//	Write element code.
		RequireIO rio{stream, requireIO};
		v.mElemCode.write(stream, kSkipRequireIO);

		//	Write just the data (no code bytes) of all the elements
		//	consecutively. Booleans constitute a special case in which we pack
		//	them 8 to a byte before writing them to the stream.
		if(v.mElemCode.baseType() == kBoolObjCode.baseType()) {
			StreamBytes(
				PackedBoolsGen(IterGen{v.mList.begin(), v.mList.end()}),
				stream, kSkipRequireIO);
		}
		else {
			for(auto&& pElem: v.mList) {
				pElem->encodeData(stream, kSkipRequireIO);
			}
		}
	}
	auto SList::DecodeData(TIStream& stream, bool requireIO) -> TValue {
		RequireIO rio{stream, requireIO};
		auto count = UIntObj::DecodeData(stream, kSkipRequireIO);
		return DecodeElems(stream, count, kSkipRequireIO);
	}
	auto SList::DecodeElems(
		TIStream& stream, TList::size_type count, bool requireIO) -> TValue
	{
		//	Read element code.
		RequireIO rio{stream, requireIO};
		auto elemCode = CodeByte::Read(stream, kSkipRequireIO);
		auto pBaseObj = FromCodeByte(elemCode);

		//	Read data of all elements consecutively.

		TValue v{elemCode, TList(count)};
		if(v.mElemCode.baseType() == kBoolObjCode.baseType()) {

			//	Special case for booleans packed 8 to a byte.
			auto byteGen = StreamedBytesGen(
				stream, (count + 7u) >> 3, kSkipRequireIO);
			v.mList.clear();
			for(auto b: UnpackedBoolsGen(std::move(byteGen), count)) {
				v.mList.push_back(std::make_shared<BoolObj>(b));
			}
		}
		else {
			for(auto&& pObj: v.mList) {
				pObj = pBaseObj->makeCopy();
				pObj->decodeData(stream, kSkipRequireIO);
			}
		}
		return std::move(v);
	}
	auto SList::typeCode() const noexcept -> CodeByte {
		return kSListCode;
	}
	void SList::assertElemTypes() const {
		AssertElemTypes(mValue);
	}
	void SList::encodeData(TOStream& stream, bool requireIO) const
	{
		EncodeData(mValue, stream, requireIO);
	}
	void SList::decodeData(TIStream& stream, bool requireIO) {
		mValue = DecodeData(stream, requireIO);
	}
	void SList::encodeElems(
		TOStream& stream, bool requireIO, bool assertTypes) const
	{
		EncodeElems(mValue, stream, requireIO, assertTypes);
	}
	void SList::decodeElems(
		TIStream& stream, TList::size_type count, bool requireIO)
	{
		mValue = DecodeElems(stream, count, requireIO);
	}
	auto SList::makeCopy(bool deep) const -> TSPBinONObj {
		if(deep) {
			return std::make_shared<SList>(
				TValue{mValue.mElemCode, DeepCopyTList(mValue.mList)});
		}
		return std::make_shared<SList>(*this);
	}
	void SList::printArgsRepr(std::ostream& stream) const {
		stream << "SListVal{";
		mValue.mElemCode.printRepr(stream);
		stream << ", ";
		PrintTListRepr(mValue.mList, stream);
		stream << '}';
	}

}
