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
		T* mPRaw;
		void retain() noexcept {
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
		
		//	A SharedPtr in an if statement (or cast to bool) will test true
		//	provided it is not the nullptr.
		explicit constexpr operator bool() const noexcept
			{ return !mPRaw; }
		
		//	get() returns the raw pointer managed by SharedPtr.
		//	Note that this may be the nullptr.	
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
		
		//	Called by operators * and -> in debug mode only.
		void assertPtr() const {
				if(!mPRaw) {
					throw NullPtrDeref{};
				}
			}
	};
	
	//	These functions should be more-or-less equivalent to the new and delete
	//	operators on scalars except they use the designated Allocator class
	//	which could potentially be customized.
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
	
	
	//	This is a convenience class template for a base class in your hierarchy
	//	that can be made polymorphic (using a trivial virtual destructor) or
	//	not based on a bool template argument.
	enum: bool { kMonomorphic, kPolymorphic };
	template<bool Morphism=kMonomorphic>
		struct BaseClass {};
	template<>
		struct BaseClass<kPolymorphic>
	{
		virtual ~BaseClass() noexcept {}
	};
	
	//	TReferenceCount gives you the suitable type for a shared object
	//	reference count depending on whether you need thread safety.
	//	The choice comes down to either std::atomic_size_t for multithreaded
	//	access or simply std::size_t for single-threaded access.
	enum: bool { kSingleThreaded, kMultithreaded };
	template<bool ThreadSafety=kSingleThreaded>
	struct ReferenceCount {
		using Type = std::size_t;
	};
	template<>
	struct ReferenceCount<kMultithreaded> {
		using Type = std::atomic_size_t;
	};
	template<typename ThreadSafety=false>
	using TReferenceCount = typename ReferenceCount<ThreadSafety>::Type;
	
	//	The Shared class template is meant to provide shareable functionality
	//	to your own classes to make them compatible with SharedPtr.
	//
	//	You inherit from it using CRTP (Curiously Recurrring Template Pattern)
	//	semantics.
	//
	//		class Foo: public Shared<Foo> {/*...*/};
	//
	//	If you have several levels of inheritance, Shared's first template
	//	argument needs to be the final class in the inheritance chain.
	//
	//		template<class Child>
	//			class Foo: public Shared<Child> {/*...*/};
	//		class Bar: public Foo<Bar> {/*...*/};
	//
	//	Shared uses an internal reference count to manage ownership, and its
	//	discard() method automatically deallocates itself which the count drops
	//	to zero. SharedPtr calls the exposed retain() and discard() methods for
	//	you automatically, so you should never have to do so yourself.
	//
	//	There are some optional boolean template arguments to consider.
	//	ThreadSafety should be set to kMultithreaded (true) if your shared
	//	pointers may be accessed by more than one thread. Morphism should be
	//	set to kPolymorphic (true) if your class is polymorphic (has virtual
	//	methods).
	//
	template<
		class Child,
		bool ThreadSafety=kSingleThreaded,
		bool Morphism=kMonomorphic,
		typename Allocator=std::allocator<Child>
		>
	class Shared: public BaseClass<Morphism> {
		mutable TReferenceCount<ThreadSafety> mRefCnt = 0;
		constexpr auto pChild() const noexcept
			{ return const_cast<Child*>(static_cast<const Child*>(this)); }
	public:
		
		//	Make() is a class method that returns a dynamically allocated
		//	instance of your class as a SharedPtr.
		//
		//		auto pFoo = Foo::Make(/* Foo constructor args... */);
		//
		//	Assuming your Foo class inherited from Shared, it should have the
		//	Make() method.
		//
		template<typename... Args>
			static auto Make(Args&&... args) {
				auto p = AllocatorNew<Child,Allocator>(
					std::forward<Args>(args)...);
				return SharedPtr<Child>{p};
			}
		
		void retain() const noexcept { ++mRefCnt; }
		void discard() const {
				if(--mRefCnt == 0) {
					AllocatorDelete<Child, Allocator>(pChild());
				}
			}
	};

}

#endif
