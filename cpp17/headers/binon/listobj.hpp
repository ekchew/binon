#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include "typeinfo.hpp"

#include <any>
#include <functional>
#include <initializer_list>
#include <optional>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace binon {

	struct TVarObj;
	struct TListObj: TStdCtnr<TListObj, std::vector<TVarObj>> {
		static constexpr auto kTypeCode = kListObjCode;
		static constexpr auto kClsName = std::string_view{"TListObj"};
		using TStdCtnr<TListObj,TValue>::TStdCtnr;
		void encodeData(TOStream& stream, bool requireIO = true) const;
		void decodeData(TIStream& stream, bool requireIO = true);
		void printArgs(std::ostream& stream) const;
	};
	struct TSList: TStdCtnr<TSList, std::vector<TVarObj>> {
		static constexpr auto kTypeCode = kSListCode;
		static constexpr auto kClsName = std::string_view{"TSList"};
		CodeByte mElemCode;
		TSList(std::any value, CodeByte elemCode = kNullObjCode);
		TSList(CodeByte elemCode = kNullObjCode);
		void encodeData(TOStream& stream, bool requireIO = true) const;
		void decodeData(TIStream& stream, bool requireIO = true);
		void printArgs(std::ostream& stream) const;
	};

	auto DeepCopyTList(const TList& list) -> TList;
	void PrintTListRepr(const TList& list, std::ostream& stream);

	struct ListBase: BinONObj {
		virtual auto list() noexcept -> TList& = 0;
		auto list() const noexcept -> const TList&
			{ return const_cast<ListBase*>(this)->list(); }
		operator TList&() noexcept { return list(); }
		operator const TList&() const noexcept { return list(); }
		auto hasValue(TList::size_type i) const -> bool;
		template<typename V> auto findValue(TList::size_type i) const
			-> std::optional<V>;
		template<typename V> auto getValue(TList::size_type i) const -> V;
		template<typename V> void setValue(TList::size_type i, V&& value);
		template<typename V> void appendValue(V&& value);
		void appendValues() {}
		template<typename V, typename... Vs>
			void appendValues(V&& v, Vs&&... vs);
	};

	struct ListObj: ListBase, AccessContainer_mValue<ListObj,TList> {
		static void EncodeData(
			const TValue& v, TOStream& stream, bool requireIO=true);
		template<typename ElemGen>
			static void EncodeElems(ElemGen elemGen,
				TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;
		static auto DecodedElemsGen(
			TIStream& stream, TValue::size_type count, bool requireIO=true) {
				return MakeGen<TSPBinONObj,RequireIO>(
					[&stream, count](RequireIO&) mutable {
						return MakeOpt<TSPBinONObj>(
							count-->0u,
							Decode, stream, kSkipRequireIO);
					},
					stream, requireIO);
			}

		/**
			MakeShared template class method:
				Allocates a new ListObj built around the arguments you supply.
				These should be simple data types of the kind typeinfo.hpp can
				handle. Internally, it calls appendValues() to populate the new
				list.

				Args:
					vs: values to go in a new list
				Returns:
					shared pointer to the new ListObj
				Example:
					You can quickly allocate a list containing an integer, a
					floating-point value, and a string with:

						auto pList = ListObj::MakeShared(42, 3.14, "foo");
		**/
		template<typename... Ts>
			static auto MakeShared(Ts&&... vs) -> std::shared_ptr<ListObj>;

		TValue mValue;

		ListObj(const TValue& v): mValue{v} {}
		ListObj(TValue&& v) noexcept: mValue{std::move(v)} {}
		ListObj() noexcept = default;
		explicit operator bool() const noexcept override
			{ return mValue.size() != 0; }
		using ListBase::list;
		auto list() noexcept -> TList& final { return mValue; }
		auto typeCode() const noexcept -> CodeByte final {return kListObjCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> std::string override
			{ return "ListObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ PrintTListRepr(mValue, stream); }
	};

	struct SListVal {
		CodeByte mElemCode;
		TList mList;
	};
	struct SList: ListBase,  AccessContainer_mValue<SList,SListVal> {
		static void AssertElemTypes(const TValue& v);
		static void EncodeData(
			const TValue& v, TOStream& stream,
			bool requireIO=true, bool assertTypes=BINON_DEBUG);
		static void EncodeElems(
			const TValue& v, TOStream& stream,
			bool requireIO=true, bool assertTypes=BINON_DEBUG);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;
		static auto DecodeElems(
			TIStream& stream, TList::size_type count,
			bool requireIO=true) -> TValue;

		TValue mValue;

		SList(CodeByte elemCode = kNullObjCode) noexcept: mValue{elemCode} {}
		SList(const TValue& v): mValue{v} {}
		SList(TValue&& v) noexcept: mValue{std::move(v)} {}
		explicit operator bool() const noexcept override
			{ return mValue.mList.size() != 0; }
		using ListBase::list;
		auto list() noexcept -> TList& final { return mValue.mList; }
		auto typeCode() const noexcept -> CodeByte final;
		void assertElemTypes() const;
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		void encodeElems(TOStream& stream,
			bool requireIO=true, bool assertTypes=BINON_DEBUG) const;
		void decodeElems(TIStream& stream, TList::size_type count,
			bool requireIO=true);
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> std::string override
			{ return "ListObj"; }
		void printArgsRepr(std::ostream& stream) const override;
	};

	//---- Low-Level Support Functions -----------------------------------------

	/**
	EncodeElems function template

	Given a generator of list elements, EncodeElems writes a code byte followed
	by the data for each element to an output stream.

	Template Args:
		T (type, required): the element type
		Gen (type, inferred)

	Args:
		gen (Gen): generator of T (or std::reference_wrapper<T>)
		stream (TOStream): output stream
		requireIO (bool, optional): throw exception on I/O error? (default=true)
	**/
	template<typename T, typename Gen>
		void EncodeElems(Gen gen, TOStream& stream, bool requireIO=true);

	/**
	DecodedElemsGen function template

	Generates element values from what it reads off an input stream.

	Template Args:
		T (type, required): the element type
			This can be either a binon type such as IntObj or a supported
			primitive type like int.

	Args:
		stream (TIStream&): binary input stream
		count (std::size_t): number of elements to read
		requireIO (bool, optional): set stream exception bits? (default=true)

	Returns:
		Generator of T:
			Note that the T elements are yielded from the Generator by value
			using move semantics.
	**/
	template<typename T>
		auto DecodedElemsGen(
			TIStream& stream, std::size_t count, bool requireIO=true)
		{
			//	The first byte on the stream should be a code byte describing
			//	the data type of all the elements that follow. Read this byte
			//	and make certain it matches what we are expecting for the
			//	template data type T.
			using TWrap = TWrapper<T>;
			CodeByte expCode = TWrap{}.typeCode();
			CodeByte actCode = CodeByte::Read(stream, requireIO);
			if(actCode.typeCode() != expCode) {
				std::ostringstream oss;
				oss << "expected BinON type code 0x" << AsHex(expCode)
					<< " but read 0x" << AsHex(actCode);
				throw TypeErr{oss.str()};
			}

			//	Now we need to return a generator that output T values. There
			//	is a special case for when T is either bool or BoolObj. In
			//	that case, the element data are packed 8 bools to a byte.
			if constexpr(std::is_same_v<TWrap, BoolObj>) {
				decltype(count) byteCnt = (count + 7u) >> 3;
				return UnpackedBoolsGen(
					StreamedBytesGen(stream, byteCnt, requireIO),
					count);
			}
			else {

				//	Otherwise, we need a single generator that decodes the
				//	data for each element one at a time.
				return MakeGen<T, RequireIO>(
					[&stream, count](RequireIO&) mutable {
						return MakeOpt<T>(
							count-->0u,
							TWrap::DecodeData, stream, kSkipRequireIO);
					}, stream, requireIO);
			}
		}

	template<typename T>
		void PrintRepr(const T& value, std::ostream& stream);

	//==== Template Implementation ============================================

	//---- ListBase -----------------------------------------------------------

	template<typename V>
		auto ListBase::findValue(TList::size_type i) const
			-> std::optional<V> {
			auto& lst = list();
			if(i < lst.size()) {
				auto pObj = lst[i];
				if(pObj) {
					return static_cast<V>(SharedObjVal<V>(pObj));
				}
			}
			return std::nullopt;
		}
	template<typename V>
		auto ListBase::getValue(TList::size_type i) const -> V {
			auto& lst = list();
			auto pObj = lst.at(i);
			if(!pObj) {
				throw NullDeref{"unallocated BinON list element"};
			}
			return static_cast<V>(SharedObjVal<V>(pObj));
		}
	template<typename V>
		void ListBase::setValue(TList::size_type i, V&& value) {
			using std::forward;
			using std::make_shared;
			auto& lst = list();
			auto& pObj = lst.at(i);
			pObj = make_shared<TWrapper<V>>(forward<V>(value));
		}
	template<typename V>
		void ListBase::appendValue(V&& value) {
			using std::forward;
			using std::make_shared;
			list().push_back(make_shared<TWrapper<V>>(forward<V>(value)));
		}
	template<typename V, typename... Vs>
		void ListBase::appendValues(V&& v, Vs&&... vs) {
			using std::forward;
			appendValue(forward<V>(v));
			appendValues(forward<Vs>(vs)...);
		}
	
	//---- ListObj ------------------------------------------------------------

	template<typename ElemGen>
		void ListObj::EncodeElems(ElemGen elemGen,
			TOStream& stream, bool requireIO)
		{
			RequireIO rio{stream, requireIO};
			for(auto& elem: elemGen) {
				elem->encode(stream, kSkipRequireIO);
			}
		}
	template<typename... Ts>
		auto ListObj::MakeShared(Ts&&... vs)
			-> std::shared_ptr<ListObj>
		{
			auto pList = std::make_shared<ListObj>();
			pList->appendValues(std::forward<Ts>(vs)...);
			return pList;
		}

	//---- Functions ----------------------------------------------------------

	template<typename T, typename Gen>
		void EncodeElems(Gen gen, TOStream& stream, bool requireIO) {
			using TWrap = TWrapper<T>;
			RequireIO rio{stream, requireIO};
			TWrap{}.typeCode().write(stream, kSkipRequireIO);
			if constexpr(std::is_same_v<TWrap, BoolObj>) {
				auto boolGen = PipeGenVals<T>(
					std::move(gen),
					[](const auto& v) { return static_cast<const T&>(v); }
					);
				StreamBytes(PackedBoolsGen(boolGen), stream, kSkipRequireIO);
			}
			else {
				for(auto&& elem: gen) {
					TWrap::EncodeData(elem, stream, kSkipRequireIO);
				}
			}
		}
	template<typename T>
		void PrintRepr(const T& value, std::ostream& stream) {

			//	If the value is already a BinON type, we can simply call its
			//	printRepr() method.
			if constexpr(kIsWrapper<T>) {
				value.printRepr(stream);
			}

			//	Otherwise, it is hopefully a primitive type we can wrap in a
			//	BinON object of some sort.
			else {
				std::ostringstream oss;
				TWrapper<T>{value}.printRepr(oss);
				auto s{std::move(oss).str()};

				//	Assuming we have made it this far, we should have a string
				//	s that reads something like "IntObj{42}", for example. But
				//	what we actually want to print is "42" in this case.
				auto i = s.find('{') + 1u;
				auto n = s.rfind('}') - i;
				stream << std::string_view{s}.substr(i, n);
			}
		}

	template<>
		struct TypeInfo<TList> {
			using Wrapper = ListObj;
			using GetType = TList;
			static auto TypeName() -> std::string { return "TList"; }
			static auto GetValue(const TSPBinONObj pObj) -> GetType {
					return BinONObj::Cast<Wrapper>(pObj)->mValue;
				}
		};
}

#endif
