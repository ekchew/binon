#ifndef BINON_SHAREDPTR_HPP
#define BINON_SHAREDPTR_HPP

//	std::shared_ptr<T> is a general purpose class template that makes few
//	assumptions about the nature of its data type T. As such, it must carry
//	around extra data with each pointer to manage a reference count.

//	binon::SharedPtr<T> assumes T provides retain() and discard() methods to
//	manage an internal reference count (or some other shared resource tracking
//	system). This reduces the burden on the pointer class, which need only be
//	a wrapper around a raw pointer to T.

//	Typically, you would inherit your custom class from SharedObj and call
//	MakeSharedPtr() to allocate it dynamically. This will set up your
//	reference counter and give your class the retain() and discard() methods
//	SharedPtr needs.

#include "macros.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#if BINON_CONCEPTS
	#include <concepts>
#endif

namespace binon {
	
	//--------------------------------------------------------------------------
	//
	//	AllocatorNew() and AllocatorDelete() should be more-or-less equivalent
	//	to the new and delete operators when used to allocate scalar objects.
	//	They give you the option to use a custom allocator, however.
	
	template<typename T, typename Allocator=std::allocator<T>, typename... Args>
		auto AllocatorNew(Args&&... args) -> T* {
			T* p;
			if constexpr(std::is_same_v<Allocator, std::allocator<T>>) {
				p = new T(std::forward<Args>(args)...);
			}
			else {
				p = new(Allocator{}.allocate(1))
			    	T(std::forward<Args>(args)...);
			}
			return p;
		}
	
	template<typename T, typename Allocator=std::allocator<T>>
		void AllocatorDelete(T* p) {
			if constexpr(std::is_same_v<Allocator, std::allocator<T>>) {
				delete p;
			}
			else {
				std::destroy_at(p);
				Allocator{}.deallocate(p, 1);
			}
		}
	
	//--------------------------------------------------------------------------
	//
	//	RefCount is a class which manages a std::size_t reference count for
	//	tracking a shareable resource. If you set its optional boolean
	//	template argument to kThreadSafe (true), it will store a
	//	std::atomic_size_t internally rather than a plain std::size_t.
	
	struct DiscardErr: std::out_of_range {
		DiscardErr():
			std::out_of_range{"discarded object with zero reference count"} {}
	};
	
	namespace details { void AssertDiscardable(std::size_t refCount); }
	
	constexpr bool kThreadSafe = true;
	
	//	Single-threaded case.
	template<bool ThreadSafe=false>
	class RefCount {
		mutable std::size_t mCount = 0;
	public:
		//	The type returned by all methods is always size_t, regardless of
		//	what is actually stored internally.
		using Type = std::size_t;
		
		static constexpr bool kThreadSafe = false;
		
		//	The reference count value can be read at any time without any
		//	explicit casting or accessor methods.
		constexpr operator Type() const noexcept { return mCount; }
		
		//	Note that both the retain() and discard() methods are marked
		//	const, event though they modify the count. This is by design.
		//	Reference counts are generally considered meta-data that can be
		//	altered even when the object on the whole is a constant.
		constexpr auto retain() const noexcept -> Type { return ++mCount; }
		
		//	In debug mode, discard() will throw a DiscardErr if the count has
		//	already reached zero.
	#if BINON_DEBUG
		auto discard() const -> Type
			{ return details::AssertDiscardable(mCount--); }
	#else
		constexpr auto discard() const noexcept -> Type { return --mCount; }
	#endif
	};
	
	//	Multithreaded case.
	template<>
	class RefCount<kThreadSafe> {
		mutable std::atomic_size_t mCount{0};
	public:
		using Type = std::size_t;
		static constexpr bool kThreadSafe = true;
		operator Type() const noexcept;
		auto retain() const noexcept -> Type;
		auto discard() const BINON_IF_RELEASE(noexcept) -> Type;
	};
	
	//--------------------------------------------------------------------------
	//
	//	SharedPtr is designed to work with any classes that implement retain()
	//	and discard() methods. Typically, you would use it with classes that
	//	inherit from SharedObj (see further down).
	//
	//	Note that SharedPtr expects the discard() method to free the object once
	//	its last reference is eliminated. In fact, SharedPtr never even looks at
	//	reference counts and such.
	
	BINON_IF_CONCEPTS(
		template<typename T> concept Shareable = requires(T v) {
			{ v.retain() };
			{ v.discard() };
		};
	)
	
	struct NullPtrDeref: std::out_of_range {
		NullPtrDeref(): std::out_of_range{"null pointer dereferenced"} {}
	};
	
	template<class T> BINON_IF_CONCEPTS(requires Shareable<T>)
	class SharedPtr
	{
		T* mPRaw;
		void retain() noexcept {
				if(mPRaw) {
					mPRaw->retain();
				}
			}
		void discard() {
				if(mPRaw) {
					mPRaw->discard();
					mPRaw = nullptr;
				}
			}
	public:
		using TRaw = T;
		
		SharedPtr(T* pRaw=nullptr) noexcept: mPRaw{pRaw} { retain(); }
		SharedPtr(const SharedPtr& sp): mPRaw{sp.mPRaw} { retain(); }
		SharedPtr(SharedPtr&& sp) noexcept: mPRaw{sp.mPRaw}
			{ sp.mPRaw = nullptr; }
		auto& operator = (const SharedPtr& sp)
			{ return *this = SharedPtr{sp}; }
		auto& operator = (SharedPtr&& sp)
			{ return std::swap(mPRaw, sp.mPRaw), *this; }
		~SharedPtr() { discard(); }
		
		//	A SharedPtr in an if statement (or cast to bool) will test true
		//	provided it is not the nullptr.
		explicit constexpr operator bool() const noexcept
			{ return !mPRaw; }
		
		//	get() returns the raw pointer managed by SharedPtr.
		//	Note that this may be the nullptr.	
		auto get() const noexcept { return mPRaw; }
		auto get() noexcept { return mPRaw; }
		
		auto& operator * () const BINON_IF_RELEASE(noexcept) {
			BINON_IF_DEBUG(assertPtr();)
			return *mPRaw;
		}
		auto& operator * () BINON_IF_RELEASE(noexcept) {
			BINON_IF_DEBUG(assertPtr();)
			return *mPRaw;
		}
		auto operator -> () const BINON_IF_RELEASE(noexcept) {
			BINON_IF_DEBUG(assertPtr();)
			return mPRaw;
			
		}
		auto operator -> () BINON_IF_RELEASE(noexcept) {
			BINON_IF_DEBUG(assertPtr();)
			return mPRaw;
		}
		
		//	Called by operators * and -> in debug mode only.
		void assertPtr() const {
				if(!mPRaw) {
					throw NullPtrDeref{};
				}
			}
	};
	
	template<
		class T, typename Allocator=std::allocator<T>,
		typename... Args
		>
		auto MakeSharedPtr(Args&&... args) -> SharedPtr<T> {
			return AllocatorNew<T,Allocator>(std::forward<Args>(args)...);
		}
	
	//--------------------------------------------------------------------------
	//
	//	SharedObj is a base class you can inherit to make your class shareable
	//	by SharedPtr. Essentially, it manages an internal RefCount and
	//	implements retain() and discard() methods for SharedPtr's sake.
	//
	//	If your class is polymorphic (uses virtual methods), the first
	//	template argument (Cls) should be the specially defined Polymorphic
	//	data type (it is by default). Otherwise, Cls should be your own class
	//	type that you allocated. For example, you could write a
	//	non-polymorphic Foo class like so:
	//
	//		class Foo: public SharedObj<Foo> {/* no virtual methods */};
	
	using Polymorphic = int;
	
	//	Non-polymorphic (monomorphic?) case.
	template<
		typename Cls=Polymorphic, bool ThreadSafe=false,
		typename Allocator=std::allocator<Cls>
		>
	class SharedObj {
		RefCount<ThreadSafe> mRefCount;
	public:
		using TClsType = Cls;
		using TAllocator = Allocator;
		using TRefCount = typename RefCount<ThreadSafe>::Type; // size_t
		static constexpr bool kThreadSafeRefCount = ThreadSafe;
		
		//	Techincally, SharedPtr does not need this method, but it's there
		//	for you anyway in case you want to inspect the reference count
		//	value for some reason?
		auto refCount() const -> TRefCount { return mRefCount; }
		
		//	SharedPtr calls these methods. As a rule, you should not!
		auto retain() const { return mRefCount.retain(); }
		auto discard() const {
				auto refCount = mRefCount.discard();
				if(refCount == 0) {
					auto pCls = static_cast<Cls*>(const_cast<SharedObj*>(this));
					AllocatorDelete<Cls,Allocator>(pCls);
				}
			}
	};
	
	//	Polymorphic case. (Note that the allocator is ingored in this case.)
	template<bool ThreadSafe>
	class SharedObj<Polymorphic, ThreadSafe, std::allocator<Polymorphic>> {
		RefCount<ThreadSafe> mRefCount;
	public:
		using TClsType = Polymorphic;
		using TRefCount = typename RefCount<ThreadSafe>::Type; // size_t
		static constexpr bool kThreadSafeRefCount = ThreadSafe;
		auto refCount() const -> TRefCount { return mRefCount; }
		auto retain() const { return mRefCount.retain(); }
		auto discard() const {
				auto refCount = mRefCount.discard();
				if(refCount == 0) {
					const_cast<SharedObj*>(this)->free();
				}
			}
		
		//	If your polymorphic class uses a custom allocator, you may need to
		//	override free() to dispose of the current instance.
		virtual void free() { delete this; }
		
		virtual ~SharedObj() {}
	};
	
}

#endif
