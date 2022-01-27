#ifndef BINON_BUFFEROBJ_HPP
#define BINON_BUFFEROBJ_HPP

#include "byteutil.hpp"
#include "hystr.hpp"
#include "intobj.hpp"
#include <ostream>

namespace binon {

	struct TBufferVal: BasicHyStr<std::byte> {
		using BasicHyStr<std::byte>::BasicHyStr;
		TBufferVal(const HyStr& hyStr);
	};
	auto operator<< (std::ostream& stream, const TBufferVal& v)
		-> std::ostream&;

	struct TBufferObj:
		TStdAcc<TBufferObj>,
		TStdEq<TBufferObj>,
		TStdHash<TBufferObj>,
		TStdCodec<TBufferObj>
	{
		using TValue = TBufferVal;
		static constexpr auto kTypeCode = kBufferObjCode;
		static constexpr auto kClsName = std::string_view{"TBufferObj"};
		TValue mValue;
		TBufferObj(const HyStr& hyStr);
		TBufferObj(TValue v);
		TBufferObj() = default;
		auto hasDefVal() const noexcept -> bool;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TBufferObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TBufferObj&;
		void printArgs(std::ostream& stream) const;
	};

	struct BufferObj: BinONObj, AccessContainer_mValue<BufferObj,TBuffer> {
		static void EncodeData(
			const TValue& v, TOStream& stream, bool requireIO=true);
		static auto DecodeData(TIStream& stream, bool requireIO=true) -> TValue;

		TValue mValue;

		BufferObj(const TValue& v): mValue{v} {}
		BufferObj(TValue&& v) noexcept: mValue{std::move(v)} {}
		BufferObj() noexcept = default;
		explicit operator bool() const noexcept override
			{ return mValue.size() != 0; }
		auto typeCode() const noexcept -> CodeByte final
			{ return kBufferObjCode; }
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto hash() const noexcept -> std::size_t;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override;
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<BufferObj>(mValue); }
		auto clsName() const noexcept -> std::string override
			{ return "BufferObj"; }
		void printArgsRepr(std::ostream& stream) const override;
	};

}

namespace std {
	template<> struct hash<binon::TBufferVal> {
		auto operator () (const binon::TBufferVal& obj) const noexcept
			-> std::size_t;
	};
	template<> struct hash<binon::BufferObj> {
		auto operator () (const binon::BufferObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
