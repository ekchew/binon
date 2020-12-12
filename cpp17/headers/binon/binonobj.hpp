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
#include <utility>
#include <vector>

namespace binon {
	class BinONObj;
	
	template<typename T> using TVector = std::vector<T, BINON_ALLOCATOR<T>>;
	using TBuffer = TVector<TStreamByte>;
	using TList = TVector<std::unique_ptr<BinONObj>>;
	
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
		
		static auto FromTypeCode(CodeByte cb) -> std::unique_ptr<BinONObj>;
		
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
		virtual auto getBuffer() const& -> const TBuffer&
			{ static TBuffer v; return typeErr(), v; }
		virtual auto getBuffer() && -> TBuffer&&
			{ static TBuffer v; return typeErr(), std::move(v); }
		virtual void setBuffer(TBuffer s) {typeErr();}
		virtual auto getStr() const& -> const TString&
			{ static TString v; return typeErr(), v; }
		virtual auto getStr() && -> TString&&
			{ static TString v; return typeErr(), std::move(v); }
		virtual void setStr(TString v) {typeErr();}
		virtual auto getList() const& -> const TList&
			{ static TList v; return typeErr(), v; }
		virtual auto getList() && -> TList&&
			{ static TList v; return typeErr(), std::move(v); }
		virtual void setList(TList v) {typeErr();}
		
		template<typename I> auto getInt() const
			{return static_cast<I>(getInt64());}
		template<typename I> void setInt(I i) {setInt64(i);}
		template<typename U> auto getUInt() const
			{return static_cast<U>(getUInt64());}
		template<typename U> void setUInt(U i) {setUInt64(i);}
		
		virtual auto typeCode() const noexcept -> CodeByte = 0;
		
		////////////////////////////////////////////////////////////////////////
		
		static auto Decode(TIStream& stream, bool requireIO=true)
			-> std::unique_ptr<BinONObj>;
		
		virtual void encode(TOStream& stream, bool requireIO=true) const;
		virtual void encodeData(TOStream& stream, bool requireIO=true) const {}
		virtual void decodeData(TIStream& stream, bool requireIO=true) {}
		virtual auto makeCopy() const -> std::unique_ptr<BinONObj> = 0;
		virtual ~BinONObj() {}
	
	protected:
		bool mHasDefVal;
		
		void typeErr() const;
	};
	
}

#endif
