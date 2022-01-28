#ifndef BINON_MIXINS_HPP
#define BINON_MIXINS_HPP

#include "codebyte.hpp"
#include "hashutil.hpp"
#include "ioutil.hpp"
#include <any>
#include <sstream>
#include <stdexcept>

namespace binon {
	struct TypeErr: std::logic_error {
		using std::logic_error::logic_error;
	};

	template<typename Child>
		struct TStdAcc {
			auto& value() & {
					return static_cast<Child*>(this)->mValue;
				}
			auto value() && {
					return std::move(static_cast<Child*>(this)->mValue);
				}
			auto& value() const& {
					return static_cast<const Child*>(this)->mValue;
				}
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
			auto encode(TOStream& stream, bool requireIO = true) const
				-> const Child&;
			auto decode(CodeByte cb, TIStream& stream, bool requireIO = true)
				-> Child&;
		};

	struct NoComparing: std::invalid_argument {
		using std::invalid_argument::invalid_argument;
	};
	struct NoHashing: std::invalid_argument {
		using std::invalid_argument::invalid_argument;
	};
	template<typename Child, typename Ctnr>
		struct TStdCtnr: TStdCodec<Child> {
			using TValue = Ctnr;
			TStdCtnr(std::any ctnr);
			TStdCtnr() = default;
			auto value() & -> TValue&;
			auto value() && -> TValue;
			auto value() const& -> const TValue&;
			auto hasDefVal() const -> bool;

			auto operator== (const TStdCtnr& rhs) const -> bool;
			auto operator!= (const TStdCtnr& rhs) const -> bool;
				 // throw NoComparing

			auto hash() const -> std::size_t; // throws NoHashing

		private:
			std::any mValue;
			auto castError() const -> TypeErr;
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
		auto TStdCodec<Child>::encode(
			TOStream& stream, bool requireIO
			) const -> const Child&
		{
			auto& child = *static_cast<const Child*>(this);
			Encode(child, stream, requireIO);
			return child;
		}
	template<typename Child>
		auto TStdCodec<Child>::decode(
			CodeByte cb, TIStream& stream, bool requireIO
			) -> Child&
		{
			auto& child = *static_cast<Child*>(this);
			Decode(child, cb, stream, requireIO);
			return child;
		}

	//--- TStdCtnr ----------------------------------------------------------

	template<typename Child, typename Ctnr>
		TStdCtnr<Child,Ctnr>::TStdCtnr(std::any ctnr):
			mValue{std::move(ctnr)}
		{
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::value() & -> TValue& {
			if(!mValue.has_value()) {
				mValue = TValue();
			}
			try {
				return std::any_cast<TValue&>(mValue);
			}
			catch(std::bad_any_cast&) {
				throw castError();
			}

		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::value() && -> TValue {
			if(!mValue.has_value()) {
				mValue = TValue();
			}
			try {
				return std::any_cast<TValue&&>(std::move(mValue));
			}
			catch(std::bad_any_cast&) {
				throw castError();
			}
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::value() const&
			-> const TValue&
		{
			return const_cast<TStdCtnr*>(this)->value();
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::operator== (const TStdCtnr&) const -> bool {
			throw NoComparing{"BinON container objects cannot be compared"};
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::operator!= (const TStdCtnr& rhs) const
			-> bool
		{
			return !(*this == rhs);
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::hash() const -> std::size_t {
			throw NoHashing{"BinON container objects cannot be hashed"};
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::hasDefVal() const -> bool {
			return value().size() == 0;
		}
	template<typename Child, typename Ctnr>
		auto TStdCtnr<Child,Ctnr>::castError() const -> TypeErr {
			std::ostringstream oss;
			oss << Child::kClsName
				<< " constructed with something other than the expected "
				<< Child::kClsName << "::TValue";
			return TypeErr{oss.str()};
		}
}

#endif
