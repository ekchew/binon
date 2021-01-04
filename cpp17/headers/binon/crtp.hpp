#ifndef BINON_CRTP_HPP
#define BINON_CRTP_HPP

//	Some CRTP (Curiously Recurring Template Pattern) utility classes inherited
//	by BinON object classes to reduce boilerplate code.

#include "codebyte.hpp"
#include "hashutil.hpp"

#include <functional>
#include <string_view>

namespace binon {
	
	template<typename Subcls, typename Value> class Read_mValue {
	public:
		using TValue = Value;
		
		constexpr operator const Value&() const noexcept
			{ return subcls().mValue; }
		constexpr operator Value&() noexcept
			{ return subcls().mValue; }
		virtual ~Read_mValue() {}
	
	protected:
		constexpr auto& subcls() noexcept
			{ return *static_cast<Subcls*>(this); }
		constexpr auto& subcls() const noexcept
			{ return *static_cast<const Subcls*>(this); }
	};
	
	template<typename Subcls, typename Value>
		class Access_mValue: public Read_mValue<Subcls,Value>
	{
	public:
		constexpr auto& operator = (const Subcls& obj) noexcept
			{ return subcls().mValue = obj.mValue, subcls(); }
		constexpr auto hash() const noexcept -> std::size_t {
				return Hash(
					std::hash<CodeByte>{}(subcls().typeCode()),
					std::hash<Value>{}(subcls().mValue));
			}
	
	protected:
		using Read_mValue<Subcls,Value>::subcls;
	};
	
	template<typename Subcls, typename Value>
		class AccessContainer_mValue: public Read_mValue<Subcls,Value>
	{
	public:
		constexpr operator Value&&() && noexcept
			{ return std::move(subcls().mValue); }
		auto& operator = (const Subcls& obj)
			{ return subcls().mValue = obj.mValue, subcls(); }
		constexpr auto& operator = (Subcls&& obj) noexcept
			{ return subcls().mValue = std::move(obj.mValue), subcls(); }
	
	protected:
		using Read_mValue<Subcls,Value>::subcls;
	};
	
}

#endif
