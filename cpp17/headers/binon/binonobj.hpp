#ifndef BINON_BINONOBJ_HPP
#define BINON_BINONOBJ_HPP

#include "codebyte.hpp"
#include "crtp.hpp"
#include "floattypes.hpp"
#include "hashutil.hpp"

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
	using TSPBinONObj = std::shared_ptr<BinONObj>;
	using TList = TVector<TSPBinONObj>;
	using TDict = std::unordered_map<TSPBinONObj, TSPBinONObj>;
	
	class NullDeref: public std::out_of_range {
	public:
		using std::out_of_range::out_of_range;
	};
	class TypeErr: public std::logic_error {
	public:
		using std::logic_error::logic_error;
	};
	
	constexpr bool kDeepCopy = true;
	
	class BinONObj {
	public:
		static auto FromCodeByte(CodeByte cb) -> TSPBinONObj;
		
		//	Returns false is object has the default value for its type or
		//	true otherwise.
		explicit virtual operator bool() const noexcept
			{ return false; }
		
		virtual auto typeCode() const noexcept -> CodeByte = 0;
		
		//	These methods are needed to support BinON objects as dictionary
		//	keys. They are supported by all types except list and dictionary
		//	variants.
		virtual auto getHash() const -> std::size_t
			{ return TypeErr{"data type cannot be hashed"}, 0; }
		virtual auto equals(const BinONObj& other) const -> bool
			{ return TypeErr{"data type cannot be compared"}, false; }
		
		static auto Decode(TIStream& stream, bool requireIO=true)
			-> TSPBinONObj;
		
		virtual void encode(TOStream& stream, bool requireIO=true) const;
		virtual void encodeData(TOStream& stream, bool requireIO=true) const {}
		virtual void decode(TIStream& stream, bool requireIO=true);
		virtual void decodeData(TIStream& stream, bool requireIO=true) {}
		virtual auto makeCopy(bool deep=false) const -> TSPBinONObj = 0;
		virtual auto clsName() const noexcept -> const char* = 0;
		virtual void printRepr(std::ostream& stream) const;
		virtual void printPtrRepr(std::ostream& stream) const;
		virtual void printArgsRepr(std::ostream& stream) const {};
		virtual ~BinONObj() {}
	};
	
	auto operator == (const TSPBinONObj& pLHS, const TSPBinONObj& pRHS) -> bool;
	auto operator << (std::ostream& stream, const BinONObj& obj)
		-> std::ostream&;
	auto operator << (std::ostream& stream, const TSPBinONObj& pObj)
		-> std::ostream&;
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
