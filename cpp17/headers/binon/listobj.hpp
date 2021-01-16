#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include "boolobj.hpp"
#include "bufferobj.hpp"
#include "floatobj.hpp"
#include "intobj.hpp"
#include "strobj.hpp"

#include <functional>
#include <initializer_list>
#include <sstream>
#include <string_view>
#include <type_traits>

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
		static void EncodeElems(
			const TValue& v, TOStream& stream, bool requireIO=true);
		template<typename ElemIt, typename EndIt>
			static void EncodeElems(ElemIt it, EndIt endIt,
				TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;
		static auto DecodeElems(TIStream& stream, TValue::size_type count,
			bool requireIO=true) -> TValue;
		static auto DecodedElemsGen(
			TIStream& stream, TValue::size_type count, bool requireIO=true) {
				return MakeGenerator<TSPBinONObj,RequireIO>(
					[&stream, count](RequireIO&) mutable
						-> std::optional<TSPBinONObj>
					{
						return MakeOptByFn<TSPBinONObj>(count--,
							[&stream] {
								return Decode(stream, kSkipRequireIO);
							});
					},
					stream, requireIO);
			}

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

		//	encodeElems() is like encodeData() except that it does not encode
		//	the length of the list. It jumps straight to encoding the
		//	elements. These methods, then, are useful if you can already tell
		//	what the length is through some other means.
		void encodeElems(TOStream& stream, bool requireIO=true) const;
		void decodeElems(TIStream& stream, TValue::size_type count,
			bool requireIO=true);

		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> std::string override
			{ return "ListObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ PrintTListRepr(mValue, stream); }
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
	};

	//	This class template helps map simple data types to their BinON wrapper
	//	class counterparts. For example, TypeInfo<bool>::Wrapper (or
	//	TWrapper<bool> for short) would be BoolObj. The class also provides a
	//	TypeName() method which, in this case, would return the std::string
	//	"bool". It is used internally by SListT and the like (see below).
	//
	//	As it stands, TypeInfo makes the following mappings of basic types:
	//
	//		Type Arg    Wrapper   Type Name
	//		________    _______    _________
	//
	//		bool        BoolObj    "bool"
	//		int8_t      IntObj     "int8_t"
	//		int16_t     IntObj     "int16_t"
	//		int32_t     IntObj     "int32_t"
	//		int64_t     IntObj     "int64_t"
	//		uint8_t     UIntObj    "uint8_t"
	//		uint16_t    UIntObj    "uint16_t"
	//		uint32_t    UIntObj    "uint32_t"
	//		uint64_t    UIntObj    "uint64_t"
	//		TFloat32    Float32Obj "TFloat32"
	//		TFloat64    FloatObj   "TFloat64"
	//		TBuffer     BufferObj  "TBuffer"
	//		std::string StrObj     "string"
	//
	//	Also, if you feed it a type is already a wrapper class, you will simply
	//	get the wrapper. In other words, TypeInfo<<BoolObj>::Wrapper is BoolObj
	//	and TypeInfo<BoolObj>::Name() is "BoolObj".
	//
	template<typename T, typename Enable=void>
	struct TypeInfo {
		static_assert(true, "BinON could not determine object type");

		//	Specializations provide:
		//		using Wrapper = ...;
		//		static auto TypeName() -> std::string;
	};
	template<typename T> using TWrapper = typename TypeInfo<T>::Wrapper;

	//	kIsWrapper<BoolObj> evaluates to true to indicate that you are already
	//	looking at a wrapper class while kIsWrapper<bool> evaluates false.
	template<typename T> inline constexpr
		bool kIsWrapper = std::is_base_of_v<BinONObj, T>;

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
		using TElemRW = std::reference_wrapper<TElem>;
		using TWrap = TWrapper<T>;
		using TCtnr = Ctnr;
		using TSize = typename Ctnr::size_type;

		static void EncodeData(
			const TCtnr& v, TOStream& stream, bool requireIO=true);
		static void EncodeElems(
			const TCtnr& v, TOStream& stream, bool requireIO=true);
		template<typename ElemIt, typename EndIt>
			static void EncodeElems(ElemIt it, EndIt endIt,
				TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TCtnr;
		static auto DecodeElems(TIStream& stream, TSize count,
			bool requireIO=true) -> TCtnr;
		/*static auto DecodedElemsGen(
			TIStream& stream, TSize count, bool requireIO=true) {
				return MakeGenerator<TElemRW,RequireIO>(
					[&stream, count](RequireIO&) mutable
						-> std::optional<TSPBinONObj>
					{
						return MakeOptByFn<TSPBinONObj>(count--,
							[&stream] {
								return TWrap::DecodeData(
									stream, kSkipRequireIO);
							});
					},
					stream, requireIO);
			}*/

		Ctnr mValue;

		SListT(std::initializer_list<TElem> lst): mValue{lst} {}
		SListT(const SList& sList);
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
		void encodeElems(TOStream& stream, bool requireIO=true) const;
		void decodeElems(TIStream& stream, TList::size_type count,
			bool requireIO=true);
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> std::string override;
		void printArgsRepr(std::ostream& stream) const override;
	};

	//==== Template Implementation ============================================

	//---- ListBase -----------------------------------------------------------

	template<typename Obj, typename... Args>
	auto ListBase::emplaceBack(Args&&... args) -> TSPBinONObj& {
		auto& lst = list();
		lst.push_back(
			std::make_shared<Obj>(std::forward<Args>(args)...));
		return lst.back();
	}

	//---- ListObj ------------------------------------------------------------

	template<typename ElemIt, typename EndIt>
	void ListObj::EncodeElems(ElemIt it, EndIt endIt,
		TOStream& stream, bool requireIO)
	{
		RequireIO rio{stream, requireIO};
		for(; it != endIt; ++it) {
			(*it)->encode(stream, kSkipRequireIO);
		}
	}

	//---- TypeInfo specializations -------------------------------------------

	template<typename T>
	struct TypeInfo<
		T, std::enable_if_t<kIsWrapper<T>>
		>
	{
		using Wrapper = T;
		static auto TypeName() -> std::string { return T{}.clsName(); }
	};
	template<typename T>
	struct TypeInfo<
		T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>
		>
	{
		using Wrapper = IntObj;
		static auto TypeName() -> std::string {
			switch(sizeof(T)) {
					case 1: return "int8_t";
					case 2: return "int16_t";
					case 4: return "int32_t";
					case 8: return "int64_t";
					default: return "SIGNED_INTEGER";
				}
			}
	};
	template<typename T>
	struct TypeInfo<T, std::enable_if_t<std::is_unsigned_v<T>>> {
		using Wrapper = UIntObj;
		static auto TypeName() -> std::string {
			switch(sizeof(T)) {
					case 1: return "uint8_t";
					case 2: return "uint16_t";
					case 4: return "uint32_t";
					case 8: return "uint64_t";
					default: return "UNSIGNED_INTEGER";
				}
			}
	};
	template<> struct TypeInfo<bool> {
		using Wrapper = BoolObj;
		static auto TypeName() -> std::string { return "bool"; }
	};
	template<> struct TypeInfo<types::TFloat32> {
		using Wrapper = Float32Obj;
		static auto TypeName() -> std::string { return "TFloat32"; }
	};
	template<> struct TypeInfo<types::TFloat64> {
		using Wrapper = FloatObj;
		static auto TypeName() -> std::string { return "TFloat64"; }
	};
	template<> struct TypeInfo<std::string> {
		using Wrapper = StrObj;
		static auto TypeName() -> std::string { return "string"; }
	};
	template<> struct TypeInfo<TBuffer> {
		using Wrapper = BufferObj;
		static auto TypeName() -> std::string { return "TBuffer"; }
	};

	//---- SListT -------------------------------------------------------------

	template<typename T, typename Ctnr>
	void SListT<T,Ctnr>::EncodeData(
		const TCtnr& v, TOStream& stream, bool requireIO)
	{
		RequireIO rio{stream, requireIO};
		UIntObj::EncodeData(v.size(), stream, kSkipRequireIO);
		EncodeElems(v, stream, kSkipRequireIO);
	}
	template<typename T, typename Ctnr>
	void SListT<T,Ctnr>::EncodeElems(
		const TCtnr& v, TOStream& stream, bool requireIO)
	{
		EncodeElems(v.begin(), v.end(), stream, requireIO);
	}
	template<typename T, typename Ctnr>
	template<typename ElemIt, typename EndIt>
	void SListT<T,Ctnr>::EncodeElems(
		ElemIt it, EndIt endIt, TOStream& stream, bool requireIO)
	{
		RequireIO rio{stream, requireIO};
		TWrap{}.typeCode().write(stream, kSkipRequireIO);
		if constexpr(std::is_same_v<TWrap, BoolObj>) {
			for(auto byt: PackedBoolsGen(it, endIt)) {
				WriteWord(byt, stream, kSkipRequireIO);
			}
		}
		else if constexpr(kIsWrapper<T>) {
			for(; it != endIt; ++it) {
				it->encodeData(stream, kSkipRequireIO);
			}
		}
		else {
			for(; it != endIt; ++it) {
				TWrap::EncodeData(*it, stream, kSkipRequireIO);
			}
		}
	}
	template<typename T, typename Ctnr>
	auto SListT<T,Ctnr>::DecodeData(TIStream& stream, bool requireIO) -> TCtnr
	{
		RequireIO rio{stream, requireIO};
		auto count = UIntObj::DecodeData(stream, kSkipRequireIO);
		return DecodeElems(stream, count, kSkipRequireIO);
	}
	template<typename T, typename Ctnr>
	auto SListT<T,Ctnr>::DecodeElems(TIStream& stream, TSize count,
		bool requireIO) -> TCtnr
	{
	}
	template<typename T, typename Ctnr>
	SListT<T,Ctnr>::SListT(const SList& sList) {
		for(auto&& p: sList.mValue.mList) {
			auto pElem = BinONObj::Cast<TWrap>(p);
			mValue.push_back(static_cast<T>(pElem->mValue));
		}
	}
	template<typename T, typename Ctnr>
	void SListT<T,Ctnr>::encodeData(TOStream& stream, bool requireIO) const {
		EncodeData(mValue, stream, requireIO);
	}
	template<typename T, typename Ctnr>
	void SListT<T,Ctnr>::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UIntObj count;
		count.decodeData(stream, kSkipRequireIO);
		decodeElems(stream, count, kSkipRequireIO);
	}
	template<typename T, typename Ctnr>
	void SListT<T,Ctnr>::encodeElems(TOStream& stream, bool requireIO) const {
		EncodeElems(mValue, stream, requireIO);
	}
	template<typename T, typename Ctnr>
	void SListT<T,Ctnr>::decodeElems(
		TIStream& stream, TList::size_type count, bool requireIO)
	{
		//	Read element code.
		RequireIO rio{stream, requireIO};
		auto code = CodeByte::Read(stream, kSkipRequireIO);
		if(code.typeCode() != TWrap{}.typeCode()) {
			std::ostringstream oss;
			oss << "expected BinON type code 0x" << AsHex(typeCode())
				<< " but read 0x" << AsHex(code.typeCode());
			throw TypeErr{oss.str()};
		}

		//	Read data of all elements consecutively.
		mValue.clear();
		if constexpr(std::is_same_v<TWrap, BoolObj>) {
			auto byteGen = MakeGenerator<std::byte>(
				[&stream] {
					return std::make_optional(
						ReadWord<std::byte>(stream, kSkipRequireIO));
				});
			for(auto b: UnpackedBoolsGen(byteGen.begin(), count)) {
				mValue.push_back(b);
			}
		}
		else {
			for(decltype(count) i = 0; i < count; ++i) {
				if constexpr(kIsWrapper<T>) {
					T obj;
					obj.decodeData(stream, kSkipRequireIO);
					mValue.push_back(obj);
				}
				else {
					TWrap obj;
					obj.decodeData(stream, kSkipRequireIO);
					mValue.push_back(static_cast<T>(obj.mValue));
				}
			}
		}
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

			if constexpr(kIsWrapper<T>) {
				elem.printRepr(stream);
			}
			else {

				//	First generate a repr string for elem using the
				//	appropriate BinON wrapper class. (Note that when we
				//	extract the string out of the ostringstream, C++20 now
				//	supports move semantics. That's why std::move() is called,
				//	but under C++17, it will still just allocate a copy of the
				//	string.)
				std::ostringstream oss;
				TWrap{elem}.printRepr(oss);
				auto s{std::move(oss).str()};

				//	At this point, we have something like StrObj{"foo"} and we
				//	want just the "foo" part, so what we print to the stream
				//	should be a string view of everything within the curly
				//	brackets.
				auto i = s.find('{') + 1u;
				auto n = s.rfind('}') - i;
				stream << std::string_view{s}.substr(i, n);
			}
		}
	}
}

#endif
