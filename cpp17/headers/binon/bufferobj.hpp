#ifndef BINON_BUFFEROBJ_HPP
#define BINON_BUFFEROBJ_HPP

#include "binonobj.hpp"

namespace binon {

	class BufferObj:
		public BinONObj, public Read_mValue<BufferObj,TBuffer>
	{
	public:
		TValue mValue;
		
		BufferObj(const TValue& v): mValue{v} {}
		BufferObj(TValue&& v) noexcept: mValue{std::forward<TValue>(v)} {}
		BufferObj() noexcept = default;
		auto& operator = (BufferObj obj)
			{ return std::swap(mValue, obj.mValue), *this; }
		auto typeCode() const noexcept -> CodeByte final
			{ return kBufferObjCode; }
		auto getBuffer() const -> const TBuffer& final {return mValue;}
		void encodeData(OStream& stream, bool requireIO=true) final;
		void decodeData(IStream& stream, bool requireIO=true) final;
		auto hash() const noexcept -> std::size_t;
	};

}

#endif
