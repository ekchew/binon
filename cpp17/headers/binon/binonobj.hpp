#ifndef BINON_BINONOBJ_HPP
#define BINON_BINONOBJ_HPP

#include "codebyte.hpp"
#include "crtp.hpp"
#include "floattypes.hpp"

#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace binon {
	
	using TBuffer = std::vector<StreamByte, BINON_ALLOCATOR<StreamByte>>;
	
	class TypeErr: public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};
	
	class BinONObj {
	public:
		constexpr auto operator == (const BinONObj&) const { return true; }
		static auto FromNullValue() -> std::unique_ptr<BinONObj>;
		static auto FromBoolValue(bool v) -> std::unique_ptr<BinONObj>;
		
		static auto FromValue() {return FromNullValue();}
		static auto FromValue(bool v) {return FromBoolValue(v);}
		
		static auto FromCodeByte(CodeByte cb) -> std::unique_ptr<BinONObj>;
		
		BinONObj(bool hasDefVal=true) noexcept: mHasDefVal{hasDefVal} {}
		
		////////////////////////////////////////////////////////////////////////
		//
		//	Subclass value accessors
		//
		//	WARNING:
		//		You must call the correct accessor for the object in question
		//		to avoid a TypeErr exception. For example, before calling
		//		getBool(), you can determine if your BinONObj is really a
		//		BoolObj with:
		//
		//			if(myObj.typeCode() == kBoolObjCode) {
		//				bool b = myObj.getBool();
		//				//...
		//			}
		//
		////////////////////////////////////////////////////////////////////////
		
		virtual auto getBool() const -> bool {return typeErr(), false;}
		virtual void setBool(bool v) { typeErr(); }
		virtual auto getInt64() const -> std::int64_t {return typeErr(), 0;}
		virtual void setInt64(std::int64_t v) { typeErr(); }
		virtual auto getUInt64() const -> std::uint64_t {return typeErr(), 0;}
		virtual void setUInt64(std::uint64_t v) {typeErr();}
		virtual auto getFloat32() const -> TFloat32 {return typeErr(), 0;}
		virtual void setFloat32(TFloat32 v) {typeErr();}
		virtual auto getFloat64() const -> TFloat64 {return typeErr(), 0;}
		virtual void setFloat64(TFloat64 v) {typeErr();}
		virtual auto getStr() const -> const String&
			{ static String v; return typeErr(), v; }
		virtual auto getBuffer() const -> const TBuffer&
			{ static TBuffer v; return typeErr(), v; }
		
		template<typename I> auto getInt() const
			{return static_cast<I>(getInt64());}
		template<typename I> void setInt(I i) {setInt64(i);}
		template<typename U> auto getUInt() const
			{return static_cast<U>(getUInt64());}
		template<typename U> void setUInt(U i) {setUInt64(i);}
		
		virtual auto typeCode() const noexcept -> CodeByte = 0;
		
		////////////////////////////////////////////////////////////////////////
		
		static auto Decode(IStream& stream, bool requireIO=true)
			-> std::unique_ptr<BinONObj>;
		
		virtual void encode(OStream& stream, bool requireIO=true);
		virtual void encodeData(OStream& stream, bool requireIO=true) {}
		virtual void decodeData(IStream& stream, bool requireIO=true) {}
		virtual ~BinONObj() {}
	
	protected:
		bool mHasDefVal;
		
		void typeErr() const;
	};
	
}

#endif
