#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include "typeinfo.hpp"

#include <functional>
#include <initializer_list>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>

namespace binon {

	auto DeepCopyTList(const TList& list) -> TList;
	void PrintTListRepr(const TList& list, std::ostream& stream);

	struct ListBase: BinONObj {
		virtual auto list() noexcept -> TList& = 0;
		auto list() const noexcept -> const TList&
			{ return const_cast<ListBase*>(this)->list(); }
		template<typename Obj, typename... Args>
			auto emplaceBack(Args&&... args) -> TSPBinONObj&;
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
				handle. Internally, it calls append() to populate the new list.

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
		auto list() noexcept -> TList& final { return mValue; }
		auto typeCode() const noexcept -> CodeByte final {return kListObjCode;}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> std::string override
			{ return "ListObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ PrintTListRepr(mValue, stream); }

		using value_type = TValue::value_type;
		using size_type = TValue::size_type;
		auto& operator [] (size_type i)
			{ return BINON_IF_DBG_REL(mValue.at(i), mValue[i]); }
		const auto& operator [] (size_type i) const
			{ return const_cast<ListObj&>(*this)[i]; }
		template<typename TObj> auto obj(size_type i) -> TObj&;
		auto begin() noexcept { return mValue.begin(); }
		auto begin() const noexcept { return mValue.begin(); }
		void clear() noexcept { mValue.clear(); }
		template<typename... Args>
			auto& emplace_back(Args&&... args)
			{ return mValue.emplace_back(std::forward<Args>(args)...); }
		auto end() noexcept { return mValue.end(); }
		auto end() const noexcept { return mValue.end(); }
		void push_back(const value_type& v) { mValue.push_back(v); }
		void push_back(value_type&& v) { mValue.push_back(std::move(v)); }
		auto size() const noexcept { return mValue.size(); }
		
		/**
			append template method:
				A higher-level alternative to push_back() that allocates a
				BinONObj appropriate to the data type you supply. Works for
				simple types typeinfo.hpp can recognize.

				Args:
					v: value to append to the current list
				Returns:
					shared pointer to the new list element
				Example:
					You can append an integer to the current list with:

						myList.append(42);
		**/
		template<typename T>
			auto append(T&& v) -> std::shared_ptr<TWrapper<T>>;

		/**
			extend template method:
				This is like append() but you can call it with multiple
				arguments, each of which will be appended to the list in the
				order given. Unlike append, it does not return anything.
				
				extend() is called internally by MakeShared(). See the example
				there to get an idea of what it can do.
				
				Args:
					vs: values to add to the current list
		**/
		void extend() {}
		template<typename T, typename... Ts>
			void extend(T&& v, Ts&&... vs);
	};

	struct SListVal {
		CodeByte mElemCode = kIntObjCode;
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

		SList(CodeByte elemCode) noexcept: mValue{elemCode} {}
		SList(const TValue& v): mValue{v} {}
		SList(TValue&& v) noexcept: mValue{std::move(v)} {}
		SList() noexcept = default;
		explicit operator bool() const noexcept override
			{ return mValue.mList.size() != 0; }
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

		using value_type = TList::value_type;
		using size_type = TList::size_type;
		auto& operator [] (size_type i)
			{ return BINON_IF_DBG_REL(mValue.mList.at(i), mValue.mList[i]); }
		const auto& operator [] (size_type i) const
			{ return const_cast<SList&>(*this)[i]; }
		template<typename TObj> auto obj(size_type i) -> TObj&;
		auto begin() noexcept { return mValue.mList.begin(); }
		auto begin() const noexcept { return mValue.mList.begin(); }
		void clear() noexcept { mValue.mList.clear(); }
		template<typename... Args>
			auto& emplace_back(Args&&... args)
			{ return mValue.mList.emplace_back(std::forward<Args>(args)...); }
		auto end() noexcept { return mValue.mList.end(); }
		auto end() const noexcept { return mValue.mList.end(); }
		void push_back(const value_type& v) { mValue.mList.push_back(v); }
		void push_back(value_type&& v) { mValue.mList.push_back(std::move(v)); }
		auto size() const noexcept { return mValue.mList.size(); }
	};

	//	This template form of SList is generally easier to use and more
	//	efficient. If you call BinONObj::Decode(), however, it will return a
	//	plain SList since Decode() cannot infer template arguments at runtime.
	//	(Note that you can SList can be cast to an SListT provided you choose an
	//	appropriate element data type.)
	//
	//	The template accepts two arguments. The first is the list element data
	//	type. This can be a BinON wrapper class like IntObj or a simple
	//	built-in type like int. (See TypeInfo above for a run-down of which
	//	types you can use.)
	//
	//	The second template argument is what container type to use. It defaults
	//	to std::vector<T>, where T is the element data type, but could be some
	//	other container like a std::deque or a std::list. It needs a size()
	//	method and to be clearable, resizeable, and iterable (though not
	//	necessarily random-access).
	//
	//	For example, you could initialize a list of integers with:
	//
	//		SListT<int> intList = {1,2,3};
	//
	//	By comparison, you would have to do something like this with an SList:
	//
	//		SList intList{kIntObjCode};
	//		intList.emplaceBack(1);
	//		intList.emplaceBack(2);
	//		intList.emplaceBack(3);
	//
	template<typename T, typename Ctnr=std::vector<T>>
		struct SListT: BinONObj {
			using TElem = T;
			using TWrap = TWrapper<T>;
			using TCtnr = Ctnr;
			using TSize = typename Ctnr::size_type;

			static void EncodeData(
				const TCtnr& v, TOStream& stream, bool requireIO=true);
			static auto DecodeData(TIStream& stream, bool requireIO=true)
				-> TCtnr;

			TCtnr mValue;

			SListT(std::initializer_list<TElem> lst): mValue{lst} {}
			SListT(const SList& sList);
			SListT(SList&& sList);
			SListT(const Ctnr& ctnr): mValue(ctnr) {}
			SListT(Ctnr&& ctnr) noexcept: mValue(std::move(ctnr)) {}
			SListT() noexcept = default;
			operator Ctnr&() noexcept { return mValue; }
			operator const Ctnr&() const noexcept { return mValue; }
			explicit operator bool() const noexcept override
				{ return mValue.size() != 0; }
			auto typeCode() const noexcept -> CodeByte final
				{ return kSListCode; }
			void encodeData(TOStream& stream, bool requireIO=true) const final;
			void decodeData(TIStream& stream, bool requireIO=true) final;
			auto makeCopy(bool deep=false) const -> TSPBinONObj override;
			auto clsName() const noexcept -> std::string override;
			void printArgsRepr(std::ostream& stream) const override;

			//---- TCtnr API ---------------------------------------------------

			using value_type = typename TCtnr::value_type;
			using size_type = typename TCtnr::size_type;
			auto& operator [] (size_type i)
				{ return BINON_IF_DBG_REL(mValue.at(i), mValue[i]); }
			const auto& operator [] (size_type i) const
				{ return const_cast<SListT&>(*this)[i]; }
			auto begin() noexcept { return mValue.begin(); }
			auto begin() const noexcept { return mValue.begin(); }
			void clear() noexcept { mValue.clear(); }
			template<typename... Args>
				auto& emplace_back(Args&&... args)
				{ return mValue.emplace_back(std::forward<Args>(args)...); }
			auto end() noexcept { return mValue.end(); }
			auto end() const noexcept { return mValue.end(); }
			void push_back(const value_type& v) { mValue.push_back(v); }
			void push_back(value_type&& v) { mValue.push_back(std::move(v)); }
			auto size() const noexcept { return mValue.size(); }
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

	template<typename Obj, typename... Args>
		auto ListBase::emplaceBack(Args&&... args) -> TSPBinONObj& {
			TList& lst = list();
			lst.push_back(
				std::make_shared<Obj>(std::forward<Args>(args)...));
			return lst.back();
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
			pList->extend(std::forward<Ts>(vs)...);
			return pList;
		}
	template<typename TObj>
		auto ListObj::obj(size_type i) -> TObj& {
			std::shared_ptr<TObj> pObj;
			auto& pBaseObj = (*this)[i];
			pObj = std::dynamic_pointer_cast<TObj>(pBaseObj);
			if(!pObj) {
				pBaseObj = pObj = std::make_shared<TObj>();
			}
			return *pObj;
		}
	template<typename T>
		auto ListObj::append(T&& v) -> std::shared_ptr<TWrapper<T>> {
			auto pObj = std::make_shared<TWrapper<T>>(std::forward<T>(v));
			push_back(pObj);
			return pObj;
		}
	template<typename T, typename... Ts>
		void ListObj::extend(T&& v, Ts&&... vs) {
			append(std::forward<T>(v));
			extend(std::forward<Ts>(vs)...);
		}

	//---- SListT -------------------------------------------------------------

	template<typename T, typename Ctnr>
		void SListT<T,Ctnr>::EncodeData(
			const TCtnr& v, TOStream& stream, bool requireIO)
		{
			RequireIO rio{stream, requireIO};
			UIntObj::EncodeData(v.size(), stream, kSkipRequireIO);
			EncodeElems<T>(IterGen{v.begin(), v.end()}, stream, kSkipRequireIO);
		}
	template<typename T, typename Ctnr>
		auto SListT<T,Ctnr>::DecodeData(TIStream& stream, bool requireIO)
			-> TCtnr
		{
			RequireIO rio{stream, requireIO};
			auto count = UIntObj::DecodeData(stream, kSkipRequireIO);
			TCtnr ctnr(count);
			ctnr.clear();
			for(T elem: DecodedElemsGen<TElem>(stream, count, kSkipRequireIO)) {
				ctnr.push_back(std::move(elem));
			}
			return std::move(ctnr);
		}
	template<typename T, typename Ctnr>
		SListT<T,Ctnr>::SListT(const SList& sList) {
			for(auto&& p0: sList.mValue.mList) {
				auto p = BinONObj::Cast<TWrap>(p0);
				mValue.push_back(static_cast<T>(p->mValue));
			}
		}
	template<typename T, typename Ctnr>
		SListT<T,Ctnr>::SListT(SList&& sList) {
			using TObjVal = typename TWrap::TValue;
			constexpr bool kValMoves =
				kIsWrapper<T> || std::is_same_v<T,TObjVal>;
			for(auto&& p0: sList.mValue.mList) {
				auto p = BinONObj::Cast<TWrap>(p0);
				if constexpr(kValMoves) {
					mValue.push_back(std::move(p->mValue));
				}
				else {
					mValue.push_back(static_cast<T>(p->mValue));
				}
			}
		}
	template<typename T, typename Ctnr>
		void SListT<T,Ctnr>::encodeData(
			TOStream& stream, bool requireIO) const
		{
			EncodeData(mValue, stream, requireIO);
		}
	template<typename T, typename Ctnr>
		void SListT<T,Ctnr>::decodeData(TIStream& stream, bool requireIO) {
			mValue = DecodeData(stream, requireIO);
		}
	template<typename T, typename Ctnr>
		auto SListT<T,Ctnr>::makeCopy(bool deep) const -> TSPBinONObj {
			return std::make_shared<SListT<T,Ctnr>>(*this);
		}
	template<typename T, typename Ctnr>
		auto SListT<T,Ctnr>::clsName() const noexcept -> std::string {
			constexpr bool kCtnrIsVector = std::is_same_v<Ctnr, std::vector<T>>;
			std::ostringstream oss;
			oss << "SListT<" << (kCtnrIsVector ? "vector" : "SEQUENCE")
				<< '<' << TypeInfo<T>::TypeName() << ">>";
			return std::move(oss).str();
		}
	template<typename T, typename Ctnr>
		void SListT<T,Ctnr>::printArgsRepr(std::ostream& stream) const {
			bool first = true;
			for(auto&& elem: mValue) {
				if(first) {
					first = false;
				}
				else {
					stream << ", ";
				}
				PrintRepr<TElem>(elem, stream);
			}
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
}

#endif
