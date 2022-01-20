#ifndef BINON_BINONOBJ_HPP
#define BINON_BINONOBJ_HPP

#include "codebyte.hpp"
#include "crtp.hpp"
#include "floattypes.hpp"
#include "hashutil.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
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

	struct NullDeref: std::out_of_range {
		using std::out_of_range::out_of_range;
	};
	struct TypeErr: std::logic_error {
		using std::logic_error::logic_error;
	};

	constexpr bool kDeepCopy = true;

	template<typename Child>
		struct TStdEqObj {
			auto operator== (const Child& rhs) const noexcept {
					return static_cast<const Child*>(this)->mValue
						== rhs.mValue;
				}
			auto operator!= (const Child& rhs) const noexcept {
					return !(*this == rhs);
				}
		};
	template<typename Child>
		struct TStdHashObj {
			auto hash() const noexcept {
					return std::hash<typename Child::TValue>{}(
						static_cast<const Child*>(this)->mValue
						);
				}
		};
	template<typename Child>
		struct TStdHasDefValObj {
			auto hasDefVal() const noexcept {
					return !static_cast<const Child*>(this)->mValue;
				}
		};
	template<typename Child>
		struct TStdPrintArgsObj {
			void printArgs(std::ostream& stream) const {
					stream << static_cast<const Child*>(this)->mValue;
				}
		};
	template<typename Child>
		struct TStdCodecObj {
			static void Encode(
				const Child& child, TOStream& stream, bool requireIO = true
				);
			static void Decode(
				Child& child, CodeByte cb, TIStream& stream,
				bool requireIO = true
				);
			void encode(TOStream& stream, bool requireIO = true) const;
			void decode(CodeByte cb, TIStream& stream, bool requireIO = true);
		};

	struct BinONObj {

		//	Cast() dynamically casts a shared pointer to a BinONObj to one of
		//	its subclasses.
		//
		//	If the Assert template argument is true, this operation must
		//	succeed. If the pointer is null, Cast() will throw NullDeref.
		//	If it has been allocated but is of a different subclass from the
		//	one you specify, it will throw TypeErr. If assertCast if false,
		//	Cast() will simply perform a static cast with no error-checking.
		//	By default, Assert is true only in debug builds.
		//
		template<typename Subcls, bool Assert=BINON_DEBUG>
			static auto Cast(const TSPBinONObj& p)
				-> std::shared_ptr<Subcls>;

		static auto Decode(TIStream& stream, bool requireIO=true)
			-> TSPBinONObj;
		static auto FromCodeByte(CodeByte cb) -> TSPBinONObj;

		//	Returns false is object has the default value for its type or
		//	true otherwise.
		explicit virtual operator bool() const noexcept { return false; }
		
		virtual auto typeCode() const noexcept -> CodeByte = 0;

		//	These methods are needed to support BinON objects as dictionary
		//	keys. They are supported by all types except list and dictionary
		//	variants.
		virtual auto getHash() const -> std::size_t
			{ return TypeErr{"data type cannot be hashed"}, 0; }
		virtual auto equals(const BinONObj& other) const -> bool
			{ return TypeErr{"data type cannot be compared"}, false; }

		virtual void encode(TOStream& stream, bool requireIO=true) const;
		virtual void encodeData(TOStream& stream, bool requireIO=true) const {}
		virtual void decode(TIStream& stream, bool requireIO=true);
		virtual void decodeData(TIStream& stream, bool requireIO=true) {}
		virtual auto makeCopy(bool deep=false) const -> TSPBinONObj = 0;
		virtual auto clsName() const noexcept -> std::string = 0;
		virtual void printRepr(std::ostream& stream) const;
		virtual void printPtrRepr(std::ostream& stream) const;
		virtual void printArgsRepr(std::ostream& stream) const {};
		virtual ~BinONObj() {}
	};

	auto operator != (const BinONObj& lhs, const BinONObj& rhs) -> bool;
	auto operator == (const BinONObj& lhs, const BinONObj& rhs) -> bool;
	auto operator == (const TSPBinONObj& pLHS, const TSPBinONObj& pRHS) -> bool;
	auto operator != (const TSPBinONObj& pLHS, const TSPBinONObj& pRHS) -> bool;
	auto operator << (std::ostream& stream, const BinONObj& obj)
		-> std::ostream&;
	auto operator << (std::ostream& stream, const TSPBinONObj& pObj)
		-> std::ostream&;

	//==== Template Implementation =============================================

	//--- TStdCodec ------------------------------------------------------------

	template<typename Child>
		void TStdCodecObj<Child>::Encode(
			const Child& child, TOStream& stream, bool requireIO
			)
		{
			RequireIO rio{stream, requireIO};
			CodeByte cb = Child::kTypeCode;
			bool hasDefVal = child.hasDefVal();
			if(hasDefVal) {
				Subtype{cb} = Subtype::kDefault;
			}
			cb.write(stream, kSkipRequireIO);
			if(!hasDefVal) {
				child.encodeData(stream, kSkipRequireIO);
			}
		}
	template<typename Child>
		void TStdCodecObj<Child>::Decode(
			Child& child, CodeByte cb, TIStream& stream, bool requireIO
			)
		{
			RequireIO rio{stream, requireIO};
			if(Subtype(cb) != Subtype::kDefault) {
				child.decodeData(cb, stream, kSkipRequireIO);
			}
		}
	template<typename Child>
		void TStdCodecObj<Child>::encode(
			TOStream& stream, bool requireIO) const
		{
			Encode(*static_cast<const Child*>(this), stream, requireIO);
		}
	template<typename Child>
		void TStdCodecObj<Child>::decode(
			CodeByte cb, TIStream& stream, bool requireIO)
		{
			Decode(*static_cast<Child*>(this), cb, stream, requireIO);
		}

	//--- BinONObj -------------------------------------------------------------

	template<typename Subcls, bool Assert>
		auto BinONObj::Cast(const TSPBinONObj& p)
			-> std::shared_ptr<Subcls>
		{
			std::shared_ptr<Subcls> pSubcls;
			if constexpr(Assert) {
				if(!p) {
					throw NullDeref{"BinONObj shared pointer unallocated"};
				}
				pSubcls = std::dynamic_pointer_cast<Subcls>(p);
				if(!pSubcls) {
				 #ifdef BINON_DEBUG
					std::cerr
						<< "Dynamic cast of BinONObj shared pointer failed.\n"
						<< "The supplied pointer looks like this:\n\t";
					p->printPtrRepr(std::cerr);
					std::cerr << '\n';
				 #endif
					throw TypeErr{
						"BinONObj shared pointer will not cast to subclass"};
				}
			}
			else {
				pSubcls = std::static_pointer_cast<Subcls>(p);
			}
			return pSubcls;
		}
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
