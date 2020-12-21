#ifndef BINON_BINONOBJ_HPP
#define BINON_BINONOBJ_HPP

#include "codebyte.hpp"
#include "crtp.hpp"
#include "floattypes.hpp"
#include "hashutil.hpp"
#include "sharedptr.hpp"

#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace binon {
	class BinONObj;
	template<typename T> using TVector = std::vector<T, BINON_ALLOCATOR<T>>;
	using TBuffer = TVector<std::byte>;
	using TSPBinONObj = SharedPtr<BinONObj>;
	using TList = TVector<TSPBinONObj>;
	using TDict = std::unordered_map<TSPBinONObj, TSPBinONObj>;
	
	class TypeErr: public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};
	
	constexpr bool kDeepCopy = true;
	
	class BinONObj: public SharedObj<Polymorphic, BINON_THREAD_SAFE> {
	public:
		static auto FromNullValue() -> TSPBinONObj;
		static auto FromBoolValue(bool v) -> TSPBinONObj;
		
		static auto FromValue() {return FromNullValue();}
		static auto FromValue(bool v) {return FromBoolValue(v);}
		
		static auto FromTypeCode(CodeByte cb) -> TSPBinONObj;
		
		constexpr auto operator == (const BinONObj&) const { return true; }
		
		//----------------------------------------------------------------------
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
		
		//	These methods are needed to support BinON objects as dictionary
		//	keys. They are supported by all types except list and dictionary
		//	variants.
		virtual auto getHash() const -> std::size_t {return typeErr(), 0;}
		virtual auto equals(const BinONObj& other) const -> bool
			{return typeErr(), false;}
		
		//----------------------------------------------------------------------
		
		static auto Decode(TIStream& stream, bool requireIO=true)
			-> TSPBinONObj;
		
		//	While the lower level Decode() returns a unique pointer to a
		//	BinONObj, DecodeType() assumes you know which BinONObj subtype to
		//	expect already and returns the IntObj or whatever it is by value.
		//	(It will throw a TypeErr if you guess wrong.)
		template<typename Subtype>
			static auto DecodeType(TIStream& stream, bool requireIO=true)
				-> Subtype
			{
				auto pBaseObj = Decode(stream, requireIO);
				auto pSubobj = dynamic_cast<Subtype*>(pBaseObj.get());
				if(!pSubobj) {
					throw TypeErr{
						"could not decode BinON object as requested type"
					};
				}
				return std::move(*pSubobj);
			}
		
		virtual void encode(TOStream& stream, bool requireIO=true) const;
		virtual void encodeData(TOStream& stream, bool requireIO=true) const {}
		virtual void decodeData(TIStream& stream, bool requireIO=true) {}
		virtual auto makeCopy(bool deep=false) const -> TSPBinONObj = 0;
		virtual auto hasDefVal() const -> bool = 0;
		virtual ~BinONObj() {}
	
	protected:
		
		void typeErr() const;
	};
	
	auto operator==(const TSPBinONObj& pLHS, const TSPBinONObj& pRHS) -> bool;
	
}

namespace std {
	template<> struct hash<binon::TSPBinONObj> {
		auto operator () (
			const binon::TSPBinONObj& pObj
			) const noexcept -> std::size_t
		{
			return pObj->getHash();
		}
	};
}

#endif
