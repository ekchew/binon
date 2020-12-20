#ifndef BINON_SHAREDPTR_HPP
#define BINON_SHAREDPTR_HPP

//	std::shared_ptr<T> is a general purpose class template that makes few
//	assumptions about the nature of its data type T. As such, it must carry
//	around extra data with each pointer to manager a reference count.
//
//	binon::SharedPtr<T> assumes its data type T provides retain() and
//	discard() methods to manage an internal reference count (or some other
//	shared resource tracking system). This reduces the burden on the pointer
//	class, which need only be a wrapper around a raw pointer to T.
//
//	Typically, you would inherit your custom class from binon::Shared and call
//	its Make() class method to allocate it dynamically. This will set up your
//	reference counter and give your class the retain() and discard() methods
//	it needs.

#include "macros.hpp"

#include <atomic>
#include <memory>
#include <new>
#include <stdexcept>
#include <string_view>
#include <utility>
#if BINON_CONCEPTS
	#include <concepts>
#endif

namespace binon {

	BINON_IF_CONCEPTS(
		template<typename T> concept Shareable = requires(T v) {
			{ v.retain() };
			{ v.discard() };
		};
	)
	
	struct NullPtrDeref: std::out_of_range {
		NullPtrDeref(): std::out_of_range{"null pointer dereferenced"} {}
	};
	
	template<typename T> BINON_IF_CONCEPTS(requires Shareable<T>)
	class SharedPtr
	{
		mutable T* mPRaw;
		constexpr void retain() const noexcept {
				if(mPRaw) {
					mPRaw->retain();
				}
			}
		void discard() const {
				if(mPRaw) {
					mPRaw->discard();
					mPRaw = nullptr;
				}
			}
	public:
		SharedPtr(T* pRaw=nullptr) noexcept: mPRaw{pRaw} { retain(); }
		SharedPtr(const SharedPtr& sp): mPRaw{p.mPRaw} { retain(); }
		constexpr SharedPtr(SharedPtr&& sp) noexcept: mPRaw{sp.mPRaw}
			{ sp.mPRaw = nullptr; }
		auto& operator = (const SharedPtr& sp)
			{ return *this = SharedPtr{sp}; }
		auto& operator = (SharedPtr&& sp)
			{ return std::swap(mPRaw, sp.mPRaw), *this; }
		~SharedPtr() { discard(); }
		explicit constexpr operator bool() const noexcept
			{ return !mPRaw; }
		constexpr auto get() const noexcept { return mPRaw; }
		constexpr auto get() noexcept { return mPRaw; }
		BINON_IF_RELEASE(constexpr)
			auto& operator * () const BINON_IF_RELEASE(noexcept) {
				BINON_IF_DEBUG(assertPtr();)
				return *mPRaw;
			}
		BINON_IF_RELEASE(constexpr)
			auto& operator * () BINON_IF_RELEASE(noexcept) {
				BINON_IF_DEBUG(assertPtr();)
				return *mPRaw;
			}
		BINON_IF_RELEASE(constexpr)
			auto operator -> () const BINON_IF_RELEASE(noexcept) {
				BINON_IF_DEBUG(assertPtr();)
				return mPRaw;
			}
		BINON_IF_RELEASE(constexpr)
			auto operator -> () BINON_IF_RELEASE(noexcept) {
				BINON_IF_DEBUG(assertPtr();)
				return mPRaw;
			}
		void assertPtr() const {
				if(!mPRaw) {
					throw NullPtrDeref{};
				}
			}
	};
	
	template<typename T, Allocator=std::allocator<T>, typename... Args>
		auto AllocatorNew(Args&&... args) {
			return new(Allocator{}.allocate(1))
			    T(std::forward<Args>(args)...);
		}
	template<typename T, Allocator=std::allocator<T>>
		void AllocatorDelete(T* p) {
			std::destroy_at(p);
			Allocator{}.deallocate(p, 1);
		}
	
	
	enum: bool { kMonomorphic, kPolymorphic };
	template<bool Morphism=kMonomorphic>
		struct BaseClass {};
	template<>
		struct BaseClass<kPolymorphic>
	{
		virtual ~BaseClass() noexcept {}
	};
	
	enum: bool { kSingleThreaded, kMultithreaded };
	template<bool Threading=kSingleThreaded>
	struct ReferenceCount {
		using Type = std::size_t;
	};
	template<>
	struct ReferenceCount<kMultithreaded> {
		using Type = std::atomic_size_t;
	};
	template<typename Threading=false>
	using TReferenceCount = typename ReferenceCount<Threading>::Type;
	
	template<
		typename Child,
		bool Threading=kSingleThreaded,
		bool Morphism=kMonomorphic,
		typename Allocator=std::allocator<Child>
		>
	class Shared: public BaseClass<Morphism> {
		mutable TReferenceCount<Threading> mRefCnt = 0;
		constexpr auto pChild() const noexcept
			{ return const_cast<Child*>(static_cast<const Child*>(this)); }
	public:
		template<typename... Args>
			static auto Make(Args&&... args) {
				auto p = AllocatorNew<Child,Allocator>(
					std::forward<Args>(args)...);
				return SharedPtr<Child>{p};
			}
		constexpr void retain() const noexcept { ++mRefCnt; }
		void discard() const {
				if(--mRefCnt == 0) {
					AllocatorDelete<Child, Allocator>(pChild());
				}
			}
	};

}

#endif
