#ifndef BINON_CRTP_HPP
#define BINON_CRTP_HPP

//	Some CRTP (Curiously Recurring Template Pattern) utility classes inherited
//	by BinON object classes to reduce boilerplate code.

#include <functional>

namespace binon {
	
	template<typename Subcls, typename Value> class Read_mValue {
	public:
		using TValue = Value;
		
		constexpr operator const Value&() const noexcept
			{ return subcls().mValue; }
		constexpr operator Value&() noexcept
			{ return subcls().mValue; }
	
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
		constexpr auto hash() const noexcept -> std::size_t
			{ return std::hash<Value>{}(subcls().mValue); }
	
	protected:
		using Read_mValue<Subcls,Value>::subcls;
	};
	
}

#endif
