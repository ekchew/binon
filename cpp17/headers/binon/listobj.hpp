#ifndef BINON_LISTOBJ_HPP
#define BINON_LISTOBJ_HPP

#include "binonobj.hpp"
#include "boolobj.hpp"
#include "bufferobj.hpp"
#include "floatobj.hpp"
#include "intobj.hpp"
#include "strobj.hpp"

#include <sstream>
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
		auto clsName() const noexcept -> const char* override
			{ return "ListObj"; }
		void printArgsRepr(std::ostream& stream) const override
			{ PrintTListRepr(mValue, stream); }
	};

	struct SListVal {
		CodeByte mElemCode = kIntObjCode;
		TList mList;
	};
	struct SList: ListBase,  AccessContainer_mValue<SList,SListVal> {
		static constexpr bool kSkipAssertTypes = false;

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
		auto clsName() const noexcept -> const char* override
			{ return "ListObj"; }
		void printArgsRepr(std::ostream& stream) const override;
	};

	template<typename T, typename Enable=void>
	struct BinON {
		static_assert(true, "BinON could not determine object type");
	};
	template<typename T> using BinONType = typename BinON<T>::type;
	template<typename T> inline constexpr auto kBinONName = BinON<T>::kName;

	template<typename Seq>
	struct SListT: BinONObj {
		using TElemObj = BinONType<typename Seq::value_type>;
		Seq mSeq;

		auto typeCode() const noexcept -> CodeByte final
			{ return TElemObj{}.typeCode(); }
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		void encodeElems(TOStream& stream, bool requireIO=true) const;
		void decodeElems(TIStream& stream, TList::size_type count,
			bool requireIO=true);
		auto makeCopy(bool deep=false) const -> TSPBinONObj override;
		auto clsName() const noexcept -> const char* override
			{ return "SListT<SEQUENCE>"; }
		void printArgsRepr(std::ostream& stream) const override;
	};

	//---- Template Implementation --------------------------------------------

	//	ListBase
	template<typename Obj, typename... Args>
	auto ListBase::emplaceBack(Args&&... args) -> TSPBinONObj& {
		auto& lst = list();
		lst.push_back(
			std::make_shared<Obj>(std::forward<Args>(args)...));
		return lst.back();
	}

	//	SListT
	template<typename Seq>
	void SListT<Seq>::encodeData(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		UIntObj count{mSeq.size()};
		count.encodeData(stream, kSkipRequireIO);
		encodeElems(stream, kSkipRequireIO);
	}
	template<typename Seq>
	void SListT<Seq>::decodeData(TIStream& stream, bool requireIO) {
		RequireIO rio{stream, requireIO};
		UIntObj count;
		count.decodeData(stream, kSkipRequireIO);
		decodeElems(stream, count, kSkipRequireIO);
	}
	template<typename Seq>
	void SListT<Seq>::encodeElems(TOStream& stream, bool requireIO) const {
		RequireIO rio{stream, requireIO};
		auto code = typeCode();
		code.write(stream, kSkipRequireIO);
		if(code.baseType() == kBoolObjCode.baseType()) {
			std::byte byt = 0x00_byte;
			std::size_t i = 0;
			for(auto elem: mSeq) {
				byt <<= 1;
				if(elem) {
					byt |= 0x01_byte;
				}
				if((++i & 0x7u) == 0x0u) {
					WriteWord(byt, stream, kSkipRequireIO);
					byt = 0x00_byte;
				}
			}
			if(i & 0x7u) {
				byt <<= 8 - i;
				WriteWord(byt, stream, kSkipRequireIO);
			}
		}
		else {
			for(auto&& elem: mSeq) {
				TElemObj{elem}.encodeData(stream, kSkipRequireIO);
			}
		}
	}
	template<typename Seq>
	void SListT<Seq>::decodeElems(
		TIStream& stream, TList::size_type count, bool requireIO)
	{
		//	Read element code.
		RequireIO rio{stream, requireIO};
		auto code = CodeByte::Read(stream, kSkipRequireIO);
		if(code.typeCode() != typeCode()) {
			std::ostringstream oss;
			oss << "expected BinON type code 0x" << AsHex(typeCode())
				<< " but read 0x" << AsHex(code.typeCode());
			throw TypeErr{oss.str()};
		}

		//	Read data of all elements consecutively.
		mSeq.clear();
		if(code.baseType() == kBoolObjCode.baseType()) {

			//	Special case for booleans packed 8 to a byte.
			std::byte byt = 0x00_byte;
			for(decltype(count) i = 0; i < count; ++i) {
				if((i & 0x7u) == 0x0u) {
					byt = ReadWord<decltype(byt)>(stream, kSkipRequireIO);
				}
				mSeq.push_back((byt & 0x80_byte) != 0x00_byte);
				byt <<= 1;
			}
		}
		else {
			for(decltype(count) i = 0; i < count; ++i) {
				TElemObj obj;
				obj.decodeData(stream, kSkipRequireIO);
				mSeq.push_back(
					static_cast<typename Seq::value_type>(obj.mValue));
			}
		}
	}
	template<typename Seq>
	auto SListT<Seq>::makeCopy(bool deep) const -> TSPBinONObj {
		return std::make_shared<SListT<Seq>>(*this);
	}
	template<typename Seq>
	void SListT<Seq>::printArgsRepr(std::ostream& stream) const {
		stream << "SEQUENCE{";
		bool first = true;
		for(auto&& elem: mSeq) {
			if(first) {
				first = false;
			}
			else {
				stream << ", ";
			}
			TElemObj{elem}.printPtrRepr(stream);
		}
		stream << "}";
	}

	//	BinON specializations.
	template<typename T>
	struct BinON<
		T, std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>
		>
	{
		using type = IntObj;
		static constexpr auto kName = "IntObj";
	};
	template<typename T>
	struct BinON<T, std::enable_if_t<std::is_unsigned_v<T>>> {
		using type = UIntObj;
		static constexpr auto kName = "UIntObj";
	};
	template<> struct BinON<bool> {
		using type = BoolObj;
		static constexpr auto kName = "BoolObj";
	};
	template<> struct BinON<types::TFloat32> {
		using type = Float32Obj;
		static constexpr auto kName = "Float32Obj";
	};
	template<> struct BinON<types::TFloat64> {
		using type = FloatObj;
		static constexpr auto kName = "FloatObj";
	};
	template<> struct BinON<std::string> {
		using type = StrObj;
		static constexpr auto kName = "StrObj";
	};
	template<> struct BinON<TBuffer> {
		using type = BufferObj;
		static constexpr auto kName = "BufferObj";
	};
}

#endif
