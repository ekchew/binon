#ifndef BINON_BINONPTR_HPP
#define BINON_BINONPTR_HPP

#include "binonobj.hpp"

namespace binon {
	
	struct NullDeref: std::out_of_range {
		using std::out_of_range::out_of_range;
	}
	
	template<typename Obj>
	class BinONPtr {
		std::shared_ptr<Obj> mSharedPtr;
	public:
		using TSharedPtr = decltype(mSharedPtr);
		
		static auto Cast(const TSPBinONObj& pObj) -> BinONPtr;
		template<typename... Args>
			static auto Make(Args&&... args) -> BinONPtr;
		
		BinONPtr(const TSharedPtr& sharedPtr): noexcept:
			mSharedPtr{sharedPtr} {}
		BinONPtr(TSharedPtr&& sharedPtr) noexcept:
			mSharedPtr{std::move(sharedPtr)} {}
		BinONPtr() noexcept = default;
		
		auto get() const noexcept { return mSharedPtr; }
		auto get() noexcept { return mSharedPtr; }
		
		explicit operator bool() const
			{ return static_cast<bool>(mSharedPtr); }
		void assertNotNull() const;
		auto operator * () const -> const Obj&;
		auto operator * () -> Obj&;
		auto operator -> () const -> const Obj*;
		auto operator -> () -> Obj*;
	};
	
	//---- IMPLEMENTATION -----------------------------------------------------
	
	template<typename Obj>
		auto BinONPtr<Obj>::Cast(const TSPBinONObj& pObj0) -> BinONPtr
	{
	#if BINON_DEBUG
		auto pObj = dynamic_pointer_cast<Obj>(pObj0);
		if(pObj0 && !pObj) {
			throw TypeErr{"BinONObj shared pointer cast to wrong type"};
		}
		return std::move(pObj);
	#else
		return static_pointer_cast<Obj>(pObj0);
	#endif
	}
	template<typename Obj> template<typename... Args>
		auto BinONPtr<Obj>::Make(Args&&... args) -> BinONPtr
	{
		return std::make_shared<Obj>(std::forward<Args>(args)...);
	}
	template<typename Obj> void BinONPtr<Obj>::assertNotNull() const {
		if(!mSharedPtr) {
			throw NullDeref{"null BinONPtr dereferenced"};
		}
	}
	template<typename Obj>
		auto BinONPtr<Obj>::operator * () const -> const Obj&
	{
		BINON_IF_DEBUG(assertNotNull();)
		return *mSharedPtr;
	}
	template<typename Obj>
		auto BinONPtr<Obj>::operator * () -> Obj&
	{
		BINON_IF_DEBUG(assertNotNull();)
		return *mSharedPtr;
	}
	template<typename Obj>
		auto BinONPtr<Obj>::operator -> () const -> const Obj*
	{
		BINON_IF_DEBUG(assertNotNull();)
		return mSharedPtr.get();
	}
	template<typename Obj>
		auto BinONPtr<Obj>::operator -> () -> Obj*
	{
		BINON_IF_DEBUG(assertNotNull();)
		return mSharedPtr.get();
	}

}

#endif
