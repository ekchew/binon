#include "binon/sharedptr.hpp"

namespace binon {
	
	namespace details {
		void AssertDiscardable(std::size_t refCount) {
			if(refCount == 0) {
				throw DiscardErr{};
			}
		}
	}
	
	RefCount<kThreadSafe>::operator Type() const noexcept {
		return mCount;
	}
	auto RefCount<kThreadSafe>::retain() const noexcept -> Type {
		return ++mCount;
	}
#if BINON_DEBUG
	auto RefCount<kThreadSafe>::release() const -> Type {
		return details::AssertDiscardable(mCount--);
	}
#else
	auto RefCount<kThreadSafe>::release() const noexcept -> Type {
		return --mCount;
	}
#endif
}
