#ifndef BINON_MIXINS_HPP
#define BINON_MIXINS_HPP

#include "codebyte.hpp"
#include "errors.hpp"
#include "hashutil.hpp"
#include "ioutil.hpp"
#include <any>
#include <sstream>

namespace binon {

	template<typename Child>
		struct StdAcc {
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
		struct StdEq {
			auto operator== (const Child& rhs) const noexcept {
					return static_cast<const Child*>(this)->mValue
						== rhs.mValue;
				}
			auto operator!= (const Child& rhs) const noexcept {
					return !(*this == rhs);
				}
		};
	template<typename Child>
		struct StdHash {
			auto hash() const noexcept {
					using TValue = typename Child::TValue;
					auto& child = *static_cast<const Child*>(this);
					auto codeHash = std::hash<CodeByte>{}(child.kTypeCode);
					auto valHash = std::hash<TValue>{}(child.mValue);
					return HashCombine(codeHash, valHash);
				}
		};
	template<typename Child>
		struct StdHasDefVal {
			auto hasDefVal() const noexcept {
					return !static_cast<const Child*>(this)->mValue;
				}
		};
	template<typename Child>
		struct StdPrintArgs {
			void printArgs(std::ostream& stream) const {
					stream << static_cast<const Child*>(this)->mValue;
				}
		};
	template<typename Child>
		struct StdCodec {
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
		struct StdCtnr: StdCodec<Child> {
			using TValue = Ctnr;
			StdCtnr(std::any ctnr);
			StdCtnr() = default;
			auto value() & -> TValue&;
			auto value() && -> TValue;
			auto value() const& -> const TValue&;
			auto hasDefVal() const -> bool;

			auto operator== (const StdCtnr& rhs) const -> bool;
			auto operator!= (const StdCtnr& rhs) const -> bool;
				 // throw NoComparing

			auto hash() const -> std::size_t; // throws NoHashing

		private:
			std::any mValue;
			auto castError() const -> TypeErr;
		};

	//==== Template Implementation =============================================

	//--- StdCodec ------------------------------------------------------------

	template<typename Child>
		void StdCodec<Child>::Encode(
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
		void StdCodec<Child>::Decode(
			Child& child, CodeByte cb, TIStream& stream, bool requireIO
			)
		{
			RequireIO rio{stream, requireIO};
			if(Subtype(cb) != Subtype::kDefault) {
				child.decodeData(stream, kSkipRequireIO);
			}
		}
	template<typename Child>
		auto StdCodec<Child>::encode(
			TOStream& stream, bool requireIO
			) const -> const Child&
		{
			auto& child = *static_cast<const Child*>(this);
			Encode(child, stream, requireIO);
			return child;
		}
	template<typename Child>
		auto StdCodec<Child>::decode(
			CodeByte cb, TIStream& stream, bool requireIO
			) -> Child&
		{
			auto& child = *static_cast<Child*>(this);
			Decode(child, cb, stream, requireIO);
			return child;
		}

	//--- StdCtnr ----------------------------------------------------------

	template<typename Child, typename Ctnr>
		StdCtnr<Child,Ctnr>::StdCtnr(std::any ctnr):
			mValue{std::move(ctnr)}
		{
		}
	template<typename Child, typename Ctnr>
		auto StdCtnr<Child,Ctnr>::value() & -> TValue& {
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
		auto StdCtnr<Child,Ctnr>::value() && -> TValue {
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
		auto StdCtnr<Child,Ctnr>::value() const&
			-> const TValue&
		{
			return const_cast<StdCtnr*>(this)->value();
		}
	template<typename Child, typename Ctnr>
		auto StdCtnr<Child,Ctnr>::operator== (const StdCtnr&) const -> bool {
			throw NoComparing{"BinON container objects cannot be compared"};
		}
	template<typename Child, typename Ctnr>
		auto StdCtnr<Child,Ctnr>::operator!= (const StdCtnr& rhs) const
			-> bool
		{
			return !(*this == rhs);
		}
	template<typename Child, typename Ctnr>
		auto StdCtnr<Child,Ctnr>::hash() const -> std::size_t {
			throw NoHashing{"BinON container objects cannot be hashed"};
		}
	template<typename Child, typename Ctnr>
		auto StdCtnr<Child,Ctnr>::hasDefVal() const -> bool {
			return value().size() == 0;
		}
	template<typename Child, typename Ctnr>
		auto StdCtnr<Child,Ctnr>::castError() const -> TypeErr {
			std::ostringstream oss;
			oss << Child::kClsName
				<< " constructed with something other than the expected "
				<< Child::kClsName << "::TValue";
			return TypeErr{oss.str()};
		}
}

#endif
