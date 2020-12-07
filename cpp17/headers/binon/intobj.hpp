#ifndef BINON_INTOBJ_HPP
#define BINON_INTOBJ_HPP

#include "binonobj.hpp"

namespace binon {
	
	class IntRangeError: public std::range_error {
	public:
		IntRangeError();
	};
	
	class IntObj: public BinONObj {
	public:
		IntObj(std::int64_t v=0) noexcept: BinONObj{v == 0}, mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kIntObjCode;}
		auto getInt64() const -> std::int64_t final {return mValue;}
		void setInt64(std::int64_t v) final {mValue = v;}
		auto getUInt64() const -> std::uint64_t final
			{ return static_cast<std::uint64_t>(mValue); }
		void setUInt64(std::uint64_t v) final
			{ mValue = static_cast<std::int64_t>(v); }
		void encodeData(OStream& stream, bool requireIO=true) final;
		void decodeData(IStream& stream, bool requireIO=true) final;
	
	private:
		std::int64_t mValue;
	};
	
	class UInt: public BinONObj {
	public:
		UInt(std::uint64_t v=0) noexcept: BinONObj{v == 0}, mValue{v} {}
		auto typeCode() const noexcept -> CodeByte final {return kUIntCode;}
		auto getUInt64() const -> std::uint64_t final {return mValue;}
		void setUInt64(std::uint64_t v) final {mValue = v;}
		void encodeData(OStream& stream, bool requireIO=true) final;
		void decodeData(IStream& stream, bool requireIO=true) final;
	
	private:
		std::uint64_t mValue;
	};

}

#endif
