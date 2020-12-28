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
		
		BufferObj(const TValue& v): mValue{v} {}
		BufferObj(TValue&& v) noexcept: mValue{std::move(v)} {}
		BufferObj(const BufferObj& v): BufferObj{v.mValue} {}
		BufferObj(BufferObj&& v) noexcept: BufferObj{std::move(v.mValue)} {}
		BufferObj() noexcept = default;
		auto operator==(const BufferObj& rhs) const noexcept -> bool;
		explicit operator bool() const noexcept override
			{ return mValue.size() != 0; }
		auto typeCode() const noexcept -> CodeByte final
			{ return kBufferObjCode; }
		void encodeData(TOStream& stream, bool requireIO=true) const final;
		void decodeData(TIStream& stream, bool requireIO=true) final;
		auto hash() const noexcept -> std::size_t;
		auto getHash() const -> std::size_t override {return hash();}
		auto equals(const BinONObj& other) const -> bool override {
				return other.typeCode() == kBufferObjCode &&
					*this == static_cast<const BufferObj&>(other);
			}
		auto makeCopy(bool deep=false) const -> TSPBinONObj override
			{ return std::make_shared<BufferObj>(mValue); }
		auto clsName() const noexcept -> const char* override
			{ return "BufferObj"; }
		void printArgsRepr(std::ostream& stream) const override;
	};

}

namespace std {
	template<> struct hash<binon::BufferObj> {
		auto operator () (const binon::BufferObj& obj) const noexcept
			-> std::size_t { return obj.hash(); }
	};
}

#endif
