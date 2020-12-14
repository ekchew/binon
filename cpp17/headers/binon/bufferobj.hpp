#ifndef BINON_BUFFEROBJ_HPP
#define BINON_BUFFEROBJ_HPP

#include "binonobj.hpp"

namespace binon {

	class BufferObj:
		public BinONObj,
		public AccessContainer_mValue<BufferObj,TBuffer>
	{
	public:
		TValue mValue;
		
		BufferObj(const TValue& v):
			BinONObj{v.size() == 0}, mValue{v} {}
		BufferObj(TValue&& v) noexcept:
			BinONObj{v.size() == 0}, mValue{std::move(v)} {}
		BufferObj(const BufferObj& v): BufferObj{v.mValue} {}
		BufferObj(BufferObj&& v) noexcept: BufferObj{std::move(v.mValue)} {}
		BufferObj() noexcept = default;
		auto typeCode() const noexcept -> CodeByte final
			{ return kBufferObjCode; }
		auto getBuffer() const& -> const TBuffer& final {return mValue;}
		auto getBuffer() && -> TBuffer&& final {return std::move(mValue);}
		void setBuffer(TBuffer v) final {std::swap(mValue, v);}
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto hash() const noexcept -> std::size_t;
		auto makeCopy() const -> std::unique_ptr<BinONObj> override
			{ return std::make_unique<BufferObj>(mValue); }
	};

}

namespace std {
	template<> struct hash<binon::BufferObj> {
		auto operator () (const binon::BufferObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
