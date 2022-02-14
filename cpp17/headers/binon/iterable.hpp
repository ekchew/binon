#include "objhelpers.hpp"

namespace binon {

	template<typename T, typename Ctnr>
		class IterBase {
		 protected:
			using TStdCtnr = typename Ctnr::TValue;
			using TStdIter = typename TStdCtnr::iterator;
			TStdCtnr* mPStdCtnr;
			TStdIter mStdIter;
			T mTemp;

			void increment();
			auto validIter() const -> bool;

		 public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = T;
			using difference_type = typename TStdIter::difference_type;
			using pointer = T*;
			using reference = T&;

			IterBase(Ctnr& ctnr, TStdIter&& stdIter) noexcept;
			auto operator* () -> reference;
			auto operator-> () -> pointer;
			virtual void load() = 0;
			virtual void flush() = 0;
			virtual ~IterBase() = default;
		};
	template<typename T, typename Ctnr>
		class ConstIterBase {
		 protected:
			using TStdCtnr = typename Ctnr::TValue;
			using TStdIter = typename TStdCtnr::const_iterator;
			const TStdCtnr* mPStdCtnr;
			TStdIter mStdIter;
			T mTemp;

			void increment();
			auto validIter() const -> bool;

		 public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = T;
			using difference_type = typename TStdIter::difference_type;
			using pointer = const T*;
			using reference = const T&;

			ConstIterBase(const Ctnr& ctnr, TStdIter&& stdIter) noexcept;
			auto operator* () const -> reference;
			auto operator-> () const -> pointer;
			virtual void load() = 0;
			virtual ~ConstIterBase() = default;
		};

	template<typename T, typename Ctnr>
		struct Iterable {
			Iterable(Ctnr);
		};
	template<typename T>
		struct Iterable<T,SList> {
			using TCtnr = SList;
			using TStdCtnr = SList::TValue;
			using TStdIter = TStdCtnr::iterator;

			struct Iter: IterBase<T,TCtnr> {
				Iter(TCtnr& ctnr, TStdIter&& stdIter);
				auto operator== (const Iter& rhs) const -> bool;
				auto operator!= (const Iter& rhs) const -> bool;
				auto operator++ () -> Iter&;
				auto operator++ (int) -> Iter;
				void load() final;
				void flush() final;
				~Iter() override;
			};

			Iterable(SList& ctnr): mPCtnr{&ctnr} {}
			auto begin() -> Iter;
			auto end() -> Iter;

		 private:
			TCtnr* mPCtnr;
		};
	template<typename T, typename Ctnr>
		struct ConstIterable {
			ConstIterable(Ctnr);
		};
	template<typename T>
		struct ConstIterable<T,SList> {
			using TCtnr = SList;
			using TStdCtnr = SList::TValue;
			using TStdIter = TStdCtnr::const_iterator;

			struct Iter: ConstIterBase<T,TCtnr> {
				Iter(const TCtnr& ctnr, TStdIter&& stdIter);
				auto operator== (const Iter& rhs) const -> bool;
				auto operator!= (const Iter& rhs) const -> bool;
				auto operator++ () -> Iter&;
				auto operator++ (int) -> Iter;
				void load() final;
			};

			ConstIterable(const SList& ctnr): mPCtnr{&ctnr} {}
			auto cbegin() const -> Iter;
			auto cend() const -> Iter;
			auto begin() const -> Iter {return cbegin();}
			auto end() const -> Iter {return cend();}

		 private:
			const TCtnr* mPCtnr;
		};

	template<typename T, typename Ctnr>
		auto AsIterable(Ctnr& ctnr) -> Iterable<T,Ctnr>;
	template<typename T, typename Ctnr>
		auto AsConstIterable(const Ctnr& ctnr) -> ConstIterable<T,Ctnr>;

	//==== Template Implementation =============================================

	//---- IterBase ------------------------------------------------------------

	template<typename T, typename C> void IterBase<T,C>::increment() {
		if(validIter()) {
			flush();
			++mStdIter;
			if(validIter()) {
				load();
			}
		}
	}
	template<typename T, typename C>
		auto IterBase<T,C>::validIter() const -> bool
	{
		return mStdIter != mPStdCtnr->end();
	}
	template<typename T, typename C>
		IterBase<T,C>::IterBase(
			C& ctnr, TStdIter&& stdIter
		) noexcept:
		mPStdCtnr{&ctnr.value()},
		mStdIter{std::forward<TStdIter>(stdIter)}
	{
	}
	template<typename T, typename C>
		auto IterBase<T,C>::operator* () -> reference
	{
		return mTemp;
	}
	template<typename T, typename C>
		auto IterBase<T,C>::operator-> () -> pointer
	{
		return &mTemp;
	}

	//---- ConstIterBase -------------------------------------------------------

	template<typename T, typename C> void ConstIterBase<T,C>::increment() {
		if(validIter()) {
			++mStdIter;
			if(validIter()) {
				load();
			}
		}
	}
	template<typename T, typename C>
		auto ConstIterBase<T,C>::validIter() const -> bool
	{
		return mStdIter != mPStdCtnr->cend();
	}
	template<typename T, typename C>
		ConstIterBase<T,C>::ConstIterBase(
			const C& ctnr, TStdIter&& stdIter
		) noexcept:
		mPStdCtnr{&ctnr.value()},
		mStdIter{std::forward<TStdIter>(stdIter)}
	{
	}
	template<typename T, typename C>
		auto ConstIterBase<T,C>::operator* () const -> reference
	{
		return mTemp;
	}
	template<typename T, typename C>
		auto ConstIterBase<T,C>::operator-> () const -> pointer
	{
		return &mTemp;
	}

	//---- Iterable ------------------------------------------------------------

	template<typename T, typename C>
		Iterable<T,C>::Iterable(C)
	{
		throw TypeErr(
			"binon::Iterable requires container type"
		);
	}

	//---- Iterable<T,SList> ---------------------------------------------------

	template<typename T>
		Iterable<T,SList>::Iter::Iter(
			TCtnr& ctnr, TStdIter&& stdIter
		):
		IterBase<T,TCtnr>{ctnr, std::forward<TStdIter>(stdIter)}
	{
		if(this->validIter()) {
			load();
		}
	}
	template<typename T>
		auto Iterable<T,SList>::Iter::operator== (const Iter& rhs) const
		-> bool
	{
		return this->mStdIter == rhs.mStdIter;
	}
	template<typename T>
		auto Iterable<T,SList>::Iter::operator!= (const Iter& rhs) const
		-> bool
	{
		return this->mStdIter != rhs.mStdIter;
	}
	template<typename T>
		auto Iterable<T,SList>::Iter::operator++ () -> Iter&
	{
		return this->increment(), *this;
	}
	template<typename T>
		auto Iterable<T,SList>::Iter::operator++ (int) -> Iter
	{
		auto copy = *this;
		return this->increment(), copy;
	}
	template<typename T> void Iterable<T,SList>::Iter::load() {
		this->mTemp = GetObjVal<T>(*this->mStdIter);
	}
	template<typename T> void Iterable<T,SList>::Iter::flush() {
		*this->mStdIter = MakeObj(this->mTemp);
	}
	template<typename T>
		Iterable<T,SList>::Iter::~Iter()
	{
		if(this->validIter()) {
			flush();
		}
	}
	template<typename T> auto Iterable<T,SList>::begin() -> Iter {
		return Iter{*mPCtnr, mPCtnr->value().begin()};
	}
	template<typename T> auto Iterable<T,SList>::end() -> Iter {
		return Iter{*mPCtnr, mPCtnr->value().end()};
	}

	//---- ConstIterable -------------------------------------------------------

	template<typename T, typename C>
		ConstIterable<T,C>::ConstIterable(C)
	{
		throw TypeErr(
			"binon::ConstIterable requires container type"
		);
	}

	//---- ConstIterable<T,SList> ----------------------------------------------

	template<typename T>
		ConstIterable<T,SList>::Iter::Iter(
			const TCtnr& ctnr, TStdIter&& stdIter
		):
		ConstIterBase<T,TCtnr>{ctnr, std::forward<TStdIter>(stdIter)}
	{
		if(this->validIter()) {
			load();
		}
	}
	template<typename T>
		auto ConstIterable<T,SList>::Iter::operator== (const Iter& rhs) const
		-> bool
	{
		return this->mStdIter == rhs.mStdIter;
	}
	template<typename T>
		auto ConstIterable<T,SList>::Iter::operator!= (const Iter& rhs) const
		-> bool
	{
		return this->mStdIter != rhs.mStdIter;
	}
	template<typename T>
		auto ConstIterable<T,SList>::Iter::operator++ () -> Iter&
	{
		return this->increment(), *this;
	}
	template<typename T>
		auto ConstIterable<T,SList>::Iter::operator++ (int) -> Iter
	{
		auto copy = *this;
		return this->increment(), copy;
	}
	template<typename T> void ConstIterable<T,SList>::Iter::load() {
		this->mTemp = GetObjVal<T>(*this->mStdIter);
	}
	template<typename T> auto ConstIterable<T,SList>::cbegin() const -> Iter {
		return Iter{*mPCtnr, mPCtnr->value().cbegin()};
	}
	template<typename T> auto ConstIterable<T,SList>::cend() const -> Iter {
		return Iter{*mPCtnr, mPCtnr->value().cend()};
	}

	//---- Helper functions ----------------------------------------------------

	template<typename T, typename Ctnr>
		auto AsIterable(Ctnr& ctnr) -> Iterable<T,Ctnr>
	{
		return Iterable<T,Ctnr>(ctnr);
	}
	template<typename T, typename Ctnr>
		auto AsConstIterable(const Ctnr& ctnr) -> ConstIterable<T,Ctnr>
	{
		return ConstIterable<T,Ctnr>(ctnr);
	}

}
