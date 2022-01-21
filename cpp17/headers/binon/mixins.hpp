#ifndef BINON_MIXINS_HPP
#define BINON_MIXINS_HPP

#include "codebyte.hpp"
#include "hashutil.hpp"
#include "ioutil.hpp"
#include <any>
#include <stdexcept>

namespace binon {
	template<typename Child>
		struct TStdAcc {
			auto& value() { static_cast<Child*>(this)->mValue; }
			auto& value() const { static_cast<const Child*>(this)->mValue; }
		};
	template<typename Child>
		struct TStdEq {
			auto operator== (const Child& rhs) const noexcept {
					return static_cast<const Child*>(this)->mValue
						== rhs.mValue;
				}
			auto operator!= (const Child& rhs) const noexcept {
					return !(*this == rhs);
				}
		};
	template<typename Child>
		struct TStdHash {
			auto hash() const noexcept {
					using TValue = typename Child::TValue;
					auto& child = *static_cast<const Child*>(this);
					auto codeHash = std::hash<CodeByte>{}(child.kTypeCode);
					auto valHash = std::hash<TValue>{}(child.mValue);
					return HashCombine(codeHash, valHash);
				}
		};
	template<typename Child>
		struct TStdHasDefVal {
			auto hasDefVal() const noexcept {
					return !static_cast<const Child*>(this)->mValue;
				}
		};
	template<typename Child>
		struct TStdPrintArgs {
			void printArgs(std::ostream& stream) const {
					stream << static_cast<const Child*>(this)->mValue;
				}
		};
	template<typename Child>
		struct TStdCodec {
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

	struct NoHashing: std::invalid_argument {
		using std::invalid_argument::invalid_argument;
	};
	template<typename Child, typename Ctnr>
		struct TStdCtnr: TStdCodec<Child> {
			using TValue = Ctnr;
			TStdCtnr(const TValue& ctnr);
			TStdCtnr(TValue&& ctnr);
			TStdCtnr();
			auto value() -> TValue&;
			auto value() const -> const TValue&;
			auto hash() const -> std::size_t; // throws NoHashing
			auto hasDefVal() const -> bool;
			
		private:
			std::any mValue;
		};

	//==== Template Implementation =============================================

	//--- TStdCodec ------------------------------------------------------------

	template<typename Child>
		void TStdCodec<Child>::Encode(
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
		void TStdCodec<Child>::Decode(
			Child& child, CodeByte cb, TIStream& stream, bool requireIO
			)
		{
			RequireIO rio{stream, requireIO};
			if(Subtype(cb) != Subtype::kDefault) {
				child.decodeData(stream, kSkipRequireIO);
			}
		}
	template<typename Child>
		void TStdCodec<Child>::encode(
			TOStream& stream, bool requireIO) const
		{
			Encode(*static_cast<const Child*>(this), stream, requireIO);
		}
	template<typename Child>
		void TStdCodec<Child>::decode(
			CodeByte cb, TIStream& stream, bool requireIO)
		{
			Decode(*static_cast<Child*>(this), cb, stream, requireIO);
		}

	//--- TStdCtnr ----------------------------------------------------------

	template<typename Child, typename Ctnr>
		TStdCtnr<Child,Ctnr>::TStdCtnr(const TValue& ctnr):
			mValue{ctnr}
		{
		}
	template<typename Child, typename Ctnr>
		TStdCtnr<Child,Ctnr>::TStdCtnr(TValue&& ctnr):
			mValue{std::forward<TValue>(ctnr)}
		{
		}
	template<typename Child, typename Ctnr>
		TStdCtnr<Child,Ctnr>::TStdCtnr():
			mValue{TValue()}
		{
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::value() -> TValue& {
			return std::any_cast<TValue&>(mValue);
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::value() const
			-> const TValue&
		{
			return std::any_cast<const TValue&>(mValue);
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::hash() const -> std::size_t {
			throw NoHashing{"BinON container objects cannot be hashed"};
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::hasDefVal() const -> bool {
			return value().size() == 0;
		}
}

#endif
