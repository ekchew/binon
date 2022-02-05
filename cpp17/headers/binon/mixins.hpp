#ifndef BINON_MIXINS_HPP
#define BINON_MIXINS_HPP

/*
Mix-in classes are used to implement functionality shared by various BinON
object classes using the CRTP (Curiously Recurring Template Pattern) paradigm.
(This is basically the newer compile-time approach to what used to be done
with virtual methods.)

Every specific BinON class implements the following minimum public interface:

	using TValue = NATIVE_DATA_TYPE;
	static const CodeByte kTypeCode = TYPE_CODE;
	static const std::string_view kClsName = CLASS_NAME; // e.g. "StrObj"
	OBJECT_TYPE(const NATIVE_DATA_TYPE&);
	OBJECT_TYPE(NATIVE_DATA_TYPE&&); // where relevant
	OBJECT_TYPE();
	auto value() & -> TValue&;
	auto value() const& -> const TValue&;
	auto value() && -> TValue; // where relevant
	auto operator== (OBJECT_TYPE) const -> bool;
	auto operator!= (OBJECT_TYPE) const -> bool;
	auto hash() const -> std::size_t;
	auto hasDefVal() const -> bool;
	auto encode(TOStream& stream, bool requireIO = true) const
		-> const OBJECT_TYPE&;
	auto decode(CodeByte cb, TIStream& stream, bool requireIO = true)
		-> OBJECT_TYPE&;
	auto encodeData(TOStream& stream, bool requireIO = true) const
		-> const OBJECT_TYPE&;
	auto decodeData(TIStream& stream, bool requireIO = true)
		-> OBJECT_TYPE&;
	void printArgs(std::ostream& stream) const;

Note that in the current implementation, operator== and operator!= will throw
NoComparing and hash() will throw NoHashing for container types.

BinONObj::print() combines kClsName with printArgs() to form the text
representation of the object.
*/

#include "codebyte.hpp"
#include "errors.hpp"
#include "hashutil.hpp"
#include "ioutil.hpp"
#include <any>
#include <sstream>

namespace binon {

	//	Builds value() methods around an mValue data member.
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

	//	Has operators == and != directly compare value()'s results on 2 objects.
	template<typename Child>
		struct StdEq {
		 protected:
			auto equals(const Child& rhs) const noexcept {
					return static_cast<const Child*>(this)->value()
						== rhs.value();
				}
		};

	//	Implements hash() method. This is done by combining the hash of the
	//	class type code with a hash of the value()'s result.
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

	//	Implements hasDefVal() method to return true if a boolean test of
	//	value()'s result evaluates false.
	template<typename Child>
		struct StdHasDefVal {
			auto hasDefVal() const noexcept {
					return !static_cast<const Child*>(this)->mValue;
				}
		};

	//	Implements the printArgs() method by simply streaming value()'s
	//	result to the output stream with the << operator.
	template<typename Child>
		struct StdPrintArgs {
			void printArgs(std::ostream& stream) const {
					stream << static_cast<const Child*>(this)->mValue;
				}
		};

	//	Implements encode() and decode() methods by calling encodeData()
	//	and decodeData() internally.
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

	/*
	The container types all share a lot of basic functionality.

	But of particular note is that the constructor accepts a std::any
	rather than the TValue type, and indeed the internally stored value is
	a std::any as well. Why, you ask? This was done to work around a
	circular header dependency problem. (For example, ListObj's value type
	is std::vector<BinONObj>. But ListObj is also a variant of BinONObj,
	and this causes the compiler a lot of drama. So the type has to be
	hidden in order for the code to compile.)

	Since the constructor does not perform a compile-time type check,
	the value() methods will instead perform a type check at run-time and
	throw binon::TypeErr if it fails.
	*/
	template<typename Child, typename Ctnr>
		struct StdCtnr: StdCodec<Child> {
			using TValue = Ctnr;
			StdCtnr(std::any ctnr);
			StdCtnr() = default;

			auto operator== (const StdCtnr& rhs) const -> bool;
			auto operator!= (const StdCtnr& rhs) const -> bool;
				 // throw NoComparing

			auto hash() const -> std::size_t; // throws NoHashing

		protected:
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
		auto StdCtnr<Child,Ctnr>::castError() const -> TypeErr {
			std::ostringstream oss;
			oss << Child::kClsName
				<< " constructed with something other than the expected "
				<< Child::kClsName << "::TValue";
			return TypeErr{oss.str()};
		}
}

#endif
