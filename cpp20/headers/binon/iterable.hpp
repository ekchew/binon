#include "objhelpers.hpp"

namespace binon {

	//	The Iterable and ConstIterable classes let you iterate over simple
	//	container types (SList, SKDict, and SDict) using more familiar data
	//	types known to the TypeConv class (see typeconv.hpp).
	//
	//	For example, you could print an SList of integers like this:
	//
	//		auto sList = MakeSList(kIntObjCode, {1, 2, 3});
	//		for(auto i: AsConstIterable<int>(sList)) {
	//			std::cout << i << '\n';
	//		}
	//
	//	Output:
	//		1
	//		2
	//		3
	//
	//	Note that only forward iterating is supported at this time.
	//
	//	Also, these classes are stricter than helper functions like
	//	GetObjVal<T>() in what they will accept as template type arguments. If
	//	you were to change the above example to AsConstIterable<unsigned int>,
	//	for example, you would get a TypeErr exception since unsigned int does
	//	not map onto kIntObjCode. In terms of the container type, there is no
	//	"free" conversion from SDict to SKDict.

	//---- IterBase class template ---------------------------------------------
	//
	//	This is the abstract base class of all iterators defined in Iterator
	//	specializations.

	template<typename T, typename Ctnr>
		class IterBase {
		 protected:
			using TStdCtnr = typename Ctnr::TValue;
			using TStdIter = typename TStdCtnr::iterator;
			TStdCtnr* mPStdCtnr;
			TStdIter mStdIter;
			T mTemp;

			void increment();

		 public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = T;
			using difference_type = typename TStdIter::difference_type;
			using pointer = T*;
			using reference = T&;

			IterBase(Ctnr& ctnr, TStdIter&& stdIter) noexcept;
			auto operator== (const IterBase& rhs) const -> bool;
			auto operator!= (const IterBase& rhs) const -> bool;
			auto operator* () -> reference;
			auto operator-> () -> pointer;

			//	When available, IterBase stores a temporary copy of the
			//	dereferenced iterator converted to type T. This is what you are
			//	accessing when you invoke the * or -> operator.
			//
			//	The load() method loads the temporary copy, while the flush()
			//	method converts it back into a BinONObj to overwrite the
			//	container element. In most cases, you would never need to call
			//	either of these manually, since the iterator logic does so
			//	automatically (e.g. as the ++ operator is invoked).
			//
			//	But if, for some reason, you need to sync the copy immediately,
			//	you can do so. A validIter() method is also available, since
			//	calling either load() or flush() on an iterator that has reached
			//	the end would elicit undefined behaviour.
			auto validIter() const -> bool;
			virtual void load() = 0;
			virtual void flush() = 0;

			virtual ~IterBase() = default;
		};

	//---- ConstIterBase class template ----------------------------------------
	//
	//	This is the abstract base class of all iterators defined in
	//	ConstIterator specializations.

	template<typename T, typename Ctnr>
		class ConstIterBase {
		 protected:
			using TStdCtnr = typename Ctnr::TValue;
			using TStdIter = typename TStdCtnr::const_iterator;
			const TStdCtnr* mPStdCtnr;
			TStdIter mStdIter;
			T mTemp;

			void increment();

		 public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = T;
			using difference_type = typename TStdIter::difference_type;
			using pointer = const T*;
			using reference = const T&;

			ConstIterBase(const Ctnr& ctnr, TStdIter&& stdIter) noexcept;
			auto operator== (const ConstIterBase& rhs) const -> bool;
			auto operator!= (const ConstIterBase& rhs) const -> bool;
			auto operator* () const -> reference;
			auto operator-> () const -> pointer;

			//	Note that while the flush() method only makes sense with
			//	IterBase and not ConstIterBase, a do-nothing implementation is
			//	still supplied here as a placeholder. This is in case you have
			//	some custom syncing logic that wants to call flush() in either
			//	case without thinking about it. (The AsIterable() helper
			//	function may return a ConstIterator when necessary, so it may
			//	not always be clear to you which kind of iterator you are
			//	working with.)
			auto validIter() const -> bool;
			virtual void load() = 0;
			constexpr void flush() const noexcept {}

			virtual ~ConstIterBase() = default;
		};

	//---- Iterable struct template --------------------------------------------
	//
	//	Iterable is designed to wrap an SList, SKDict, or SDict in an object
	//	that is made iterable by exposing begin() and end() methods. Internally,
	//	it stores a pointer to the container you pass into its constructor, so
	//	the original container must exist for at least as long as the Iterable
	//	does to avoid undefined behaviour.
	//
	//	It is specialized for each of the 3 container types, and the default
	//	general implementation is actually illegal (similar to the TypeConv
	//	class). Its constructor will throw a NonCtnrType.
	//
	//	There is an AsIterable<T>() helper function which you would typically
	//	call to instantiate this struct.

	template<typename T, typename Ctnr>
		struct Iterable {
			Iterable(Ctnr); // throws NonCtnrType
		};

	//---- ConstIterable struct template ---------------------------------------
	//
	//	Unlike the traditional approach of having a custom class supply both a
	//	mutable and constant iterator type, there is a whole separate
	//	ConstIterable class. Both Iterable and ConstIterable only implement a
	//	single iterator type (called Iter in both cases), but of course the
	//	ConstIterable version does not let you modify dereferenced iterator
	//	values.

	template<typename T, typename Ctnr>
		struct ConstIterable {
			ConstIterable(Ctnr); // throws NonCtnrType
		};

	//---- Helper functions ----------------------------------------------------
	//
	//	AsIterable<T>() and AsConstIterable<T>() are helper functions that
	//	return an Iterable<T,Ctnr> or ConstIterable<T,Ctnr> built around an
	//	SList, SKDict, or SDict object you pass in. Note that
	//	AsIterable<T>(ctnr) may return a ConstIterable<T,Ctnr> if your ctnr
	//	argument is a constant.

	template<typename T, typename Ctnr>
		auto AsIterable(Ctnr& ctnr) -> Iterable<T,Ctnr>;
	template<typename T, typename Ctnr>
		auto AsIterable(const Ctnr& ctnr) -> ConstIterable<T,Ctnr>;
	template<typename T, typename Ctnr>
		auto AsConstIterable(const Ctnr& ctnr) -> ConstIterable<T,Ctnr>;

	//---- SList Iterable and ConstIterable specializations --------------------
	//
	//	This specialization requires a type T template argument that maps onto
	//	the element code type (mElemCode) of the SList you supply by
	//	constructor.
	//
	//	Example:
	//
	//		auto sList = MakeSList(kUIntCode, {1U, 2U, 3U});
	//		for(auto& i: Iterable<unsigned int>(sList)) {
	//			i *= i;
	//		}
	//		for(auto i: ConstIterable<unsigned int>(sList)) {
	//			std::cout << i << '\n';
	//		}
	//
	//	Output:
	//		1
	//		4
	//		9

	template<typename T>
		struct Iterable<T,SList> {
			using TValue = T;
			using TCtnr = SList;
			using TStdCtnr = TCtnr::TValue;
			using TStdIter = TStdCtnr::iterator;

			struct Iter: IterBase<TValue,TCtnr> {
				Iter(TCtnr& ctnr, TStdIter&& stdIter);
				auto operator++ () -> Iter&;
				auto operator++ (int) -> Iter;
				void load() final;
				void flush() final;
				~Iter() override;
			};

			Iterable(TCtnr& ctnr);
			auto begin() -> Iter;
			auto end() -> Iter;

		 private:
			TCtnr* mPCtnr;
		};
	template<typename T>
		struct ConstIterable<T,SList> {
			using TValue = T;
			using TCtnr = SList;
			using TStdCtnr = TCtnr::TValue;
			using TStdIter = TStdCtnr::const_iterator;

			struct Iter: ConstIterBase<TValue,TCtnr> {
				Iter(const TCtnr& ctnr, TStdIter&& stdIter);
				auto operator++ () -> Iter&;
				auto operator++ (int) -> Iter;
				void load() final;
			};

			ConstIterable(const TCtnr& ctnr);
			auto begin() const -> Iter;
			auto end() const -> Iter;

		 private:
			const TCtnr* mPCtnr;
		};

	//---- SKDict Iterable and ConstIterable specializations -------------------
	//
	//	With an SKDict, you supply a type that maps onto its key. The iterator
	//	then produces pairs of Key,BinONObj. The flush() method for Iterable
	//	will only update the BinONObj value since the keys are constant in
	//	unordered_map iterators.

	template<typename Key>
		struct Iterable<Key,SKDict> {
			using TValue = std::pair<Key,BinONObj>;
			using TCtnr = SKDict;
			using TStdCtnr = TCtnr::TValue;
			using TStdIter = TStdCtnr::iterator;

			struct Iter: IterBase<TValue,TCtnr> {
				Iter(TCtnr& ctnr, TStdIter&& stdIter);
				auto operator++ () -> Iter&;
				auto operator++ (int) -> Iter;
				void load() final;
				void flush() final;
				~Iter() override;
			};

			Iterable(TCtnr& ctnr);
			auto begin() -> Iter;
			auto end() -> Iter;

		 private:
			TCtnr* mPCtnr;
		};
	template<typename Key>
		struct ConstIterable<Key,SKDict> {
			using TValue = std::pair<Key,BinONObj>;
			using TCtnr = SKDict;
			using TStdCtnr = TCtnr::TValue;
			using TStdIter = TStdCtnr::const_iterator;

			struct Iter: ConstIterBase<TValue,TCtnr> {
				Iter(const TCtnr& ctnr, TStdIter&& stdIter);
				auto operator++ () -> Iter&;
				auto operator++ (int) -> Iter;
				void load() final;
			};

			ConstIterable(const TCtnr& ctnr);
			auto begin() const -> Iter;
			auto end() const -> Iter;

		 private:
			const TCtnr* mPCtnr;
		};

	//---- SDict Iterable and ConstIterable specializations --------------------
	//
	//	With an SDict, you provide both key and value types that must map to the
	//	corresponding type codes in the SDict object. With the AsIterable() and
	//	AsConstIterable() methods, you need to pack the two types into a
	//	std::pair.
	//
	//	Example:
	//
	//		using std::cout;
	//		using std::pair;
	//		using std::string_view;
	//		auto sDict = MakeSDict(
	//			kStrObjCode, kIntObjCode,
	//			{{"foo", 1}, {"bar", 2}, {"baz", 3}}
	//		);
	//		for(auto& pair: AsIterable<pair<string_view,int>>(sDict)) {
	//			pair.second *= pair.second;
	//		}
	//		for(auto& pair: AsConstIterable<pair<string_view,int>>(sDict)) {
	//			cout << pair.first << ": " << pair.second << '\n';
	//		}
	//
	//	Possible output:
	//
	//		bar: 4
	//		baz: 9
	//		foo: 1

	template<typename Key, typename Val>
		struct Iterable<std::pair<Key,Val>,SDict> {
			using TValue = std::pair<Key,Val>;
			using TCtnr = SDict;
			using TStdCtnr = TCtnr::TValue;
			using TStdIter = TStdCtnr::iterator;

			struct Iter: IterBase<TValue,TCtnr> {
				Iter(TCtnr& ctnr, TStdIter&& stdIter);
				auto operator++ () -> Iter&;
				auto operator++ (int) -> Iter;
				void load() final;
				void flush() final;
				~Iter() override;
			};

			Iterable(TCtnr& ctnr);
			auto begin() -> Iter;
			auto end() -> Iter;

		 private:
			TCtnr* mPCtnr;
		};
	template<typename Key, typename Val>
		struct ConstIterable<std::pair<Key,Val>,SDict> {
			using TValue = std::pair<Key,Val>;
			using TCtnr = SDict;
			using TStdCtnr = TCtnr::TValue;
			using TStdIter = TStdCtnr::const_iterator;

			struct Iter: ConstIterBase<TValue,TCtnr> {
				Iter(const TCtnr& ctnr, TStdIter&& stdIter);
				auto operator++ () -> Iter&;
				auto operator++ (int) -> Iter;
				void load() final;
			};

			ConstIterable(const TCtnr& ctnr);
			auto begin() const -> Iter;
			auto end() const -> Iter;

		 private:
			const TCtnr* mPCtnr;
		};

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
		IterBase<T,C>::IterBase(
			C& ctnr, TStdIter&& stdIter
		) noexcept:
		mPStdCtnr{&ctnr.value()},
		mStdIter{std::forward<TStdIter>(stdIter)}
	{
	}
	template<typename T, typename C>
		auto IterBase<T,C>::operator== (const IterBase& rhs) const
		-> bool
	{
		return mStdIter == rhs.mStdIter;
	}
	template<typename T, typename C>
		auto IterBase<T,C>::operator!= (const IterBase& rhs) const
		-> bool
	{
		return mStdIter != rhs.mStdIter;
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
	template<typename T, typename C>
		auto IterBase<T,C>::validIter() const -> bool
	{
		return mStdIter != mPStdCtnr->end();
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
		ConstIterBase<T,C>::ConstIterBase(
			const C& ctnr, TStdIter&& stdIter
		) noexcept:
		mPStdCtnr{&ctnr.value()},
		mStdIter{std::forward<TStdIter>(stdIter)}
	{
	}
	template<typename T, typename C>
		auto ConstIterBase<T,C>::operator== (const ConstIterBase& rhs) const
		-> bool
	{
		return mStdIter == rhs.mStdIter;
	}
	template<typename T, typename C>
		auto ConstIterBase<T,C>::operator!= (const ConstIterBase& rhs) const
		-> bool
	{
		return mStdIter != rhs.mStdIter;
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
	template<typename T, typename C>
		auto ConstIterBase<T,C>::validIter() const -> bool
	{
		return mStdIter != mPStdCtnr->cend();
	}

	//---- Iterable ------------------------------------------------------------

	template<typename T, typename C>
		Iterable<T,C>::Iterable(C)
	{
		throw NonCtnrType{
			"binon::Iterable requires container type"
		};
	}

	//---- ConstIterable -------------------------------------------------------

	template<typename T, typename C>
		ConstIterable<T,C>::ConstIterable(C)
	{
		throw NonCtnrType{
			"binon::ConstIterable requires container type"
		};
	}

	//---- Helper functions ----------------------------------------------------

	template<typename T, typename Ctnr>
		auto AsIterable(Ctnr& ctnr) -> Iterable<T,Ctnr>
	{
		return Iterable<T,Ctnr>(ctnr);
	}
	template<typename T, typename Ctnr>
		auto AsIterable(const Ctnr& ctnr) -> ConstIterable<T,Ctnr>
	{
		return ConstIterable<T,Ctnr>(ctnr);
	}
	template<typename T, typename Ctnr>
		auto AsConstIterable(const Ctnr& ctnr) -> ConstIterable<T,Ctnr>
	{
		return ConstIterable<T,Ctnr>(ctnr);
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
	template<typename T>
		Iterable<T,SList>::Iterable(TCtnr& ctnr): mPCtnr{&ctnr}
	{
		if(TGetObj<T>::kTypeCode != ctnr.mElemCode) {
			throw BadIterType{
				"Iterable type T does not map to SList element code"
			};
		}
	}
	template<typename T> auto Iterable<T,SList>::begin() -> Iter {
		return Iter{*mPCtnr, mPCtnr->value().begin()};
	}
	template<typename T> auto Iterable<T,SList>::end() -> Iter {
		return Iter{*mPCtnr, mPCtnr->value().end()};
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
	template<typename T>
		ConstIterable<T,SList>::ConstIterable(const TCtnr& ctnr): mPCtnr{&ctnr}
	{
		if(TGetObj<T>::kTypeCode != ctnr.mElemCode) {
			throw BadIterType{
				"ConstIterable type T does not map to SList element code"
			};
		}
	}
	template<typename T> auto ConstIterable<T,SList>::begin() const -> Iter {
		return Iter{*mPCtnr, mPCtnr->value().cbegin()};
	}
	template<typename T> auto ConstIterable<T,SList>::end() const -> Iter {
		return Iter{*mPCtnr, mPCtnr->value().cend()};
	}

	//---- Iterable<T,SKDict> --------------------------------------------------

	template<typename K>
		Iterable<K,SKDict>::Iter::Iter(
			TCtnr& ctnr, TStdIter&& stdIter
		):
		IterBase<TValue,TCtnr>{ctnr, std::forward<TStdIter>(stdIter)}
	{
		if(this->validIter()) {
			load();
		}
	}
	template<typename K>
		auto Iterable<K,SKDict>::Iter::operator++ ()
		-> Iter&
	{
		return this->increment(), *this;
	}
	template<typename K>
		auto Iterable<K,SKDict>::Iter::operator++ (int)
		-> Iter
	{
		auto copy = *this;
		return this->increment(), copy;
	}
	template<typename K>
		void Iterable<K,SKDict>::Iter::load()
	{
		this->mTemp = std::make_pair(
			GetObjVal<K>(this->mStdIter->first),
			this->mStdIter->second
		);
	}
	template<typename K>
		void Iterable<K,SKDict>::Iter::flush()
	{
		this->mStdIter->second = this->mTemp->second;
	}
	template<typename K>
		Iterable<K,SKDict>::Iter::~Iter()
	{
		if(this->validIter()) {
			flush();
		}
	}
	template<typename K>
		Iterable<K,SKDict>::Iterable(TCtnr& ctnr):
			mPCtnr{&ctnr}
	{
		if(TGetObj<K>::kTypeCode != ctnr.mKeyCode) {
			throw BadIterType{
				"Iterable key type does not map to SKDict key code"
			};
		}
	}
	template<typename K>
		auto Iterable<K,SKDict>::begin() -> Iter
	{
		return Iter{*mPCtnr, mPCtnr->value().begin()};
	}
	template<typename K>
		auto Iterable<K,SKDict>::end() -> Iter
	{
		return Iter{*mPCtnr, mPCtnr->value().end()};
	}

	//---- ConstIterable<T,SKDict> ---------------------------------------------

	template<typename K>
		ConstIterable<K,SKDict>::Iter::Iter(
			const TCtnr& ctnr, TStdIter&& stdIter
		):
		ConstIterBase<TValue,TCtnr>{ctnr, std::forward<TStdIter>(stdIter)}
	{
		if(this->validIter()) {
			load();
		}
	}
	template<typename K>
		auto ConstIterable<K,SKDict>::Iter::operator++ ()
		-> Iter&
	{
		return this->increment(), *this;
	}
	template<typename K>
		auto ConstIterable<K,SKDict>::Iter::operator++ (int)
		-> Iter
	{
		auto copy = *this;
		return this->increment(), copy;
	}
	template<typename K>
		void ConstIterable<K,SKDict>::Iter::load()
	{
		this->mTemp = std::make_pair(
			GetObjVal<K>(this->mStdIter->first),
			this->mStdIter->second
		);
	}
	template<typename K>
		ConstIterable<K,SKDict>::ConstIterable(
			const TCtnr& ctnr
		):
		mPCtnr{&ctnr}
	{
		if(TGetObj<K>::kTypeCode != ctnr.mKeyCode) {
			throw BadIterType{
				"ConstIterable key type does not map to SKDict key code"
			};
		}
	}
	template<typename K>
		auto ConstIterable<K,SKDict>::begin() const -> Iter
	{
		return Iter{*mPCtnr, mPCtnr->value().cbegin()};
	}
	template<typename K>
		auto ConstIterable<K,SKDict>::end() const -> Iter
	{
		return Iter{*mPCtnr, mPCtnr->value().cend()};
	}

	//---- Iterable<T,SDict> --------------------------------------------------

	template<typename K, typename V>
		Iterable<std::pair<K,V>,SDict>::Iter::Iter(
			TCtnr& ctnr, TStdIter&& stdIter
		):
		IterBase<TValue,TCtnr>{ctnr, std::forward<TStdIter>(stdIter)}
	{
		if(this->validIter()) {
			load();
		}
	}
	template<typename K, typename V>
		auto Iterable<std::pair<K,V>,SDict>::Iter::operator++ ()
		-> Iter&
	{
		return this->increment(), *this;
	}
	template<typename K, typename V>
		auto Iterable<std::pair<K,V>,SDict>::Iter::operator++ (int)
		-> Iter
	{
		auto copy = *this;
		return this->increment(), copy;
	}
	template<typename K, typename V>
		void Iterable<std::pair<K,V>,SDict>::Iter::load()
	{
		this->mTemp = std::make_pair(
			GetObjVal<K>(this->mStdIter->first),
			GetObjVal<V>(this->mStdIter->second)
		);
	}
	template<typename K, typename V>
		void Iterable<std::pair<K,V>,SDict>::Iter::flush()
	{
		this->mStdIter->second = MakeObj(this->mTemp.second);
	}
	template<typename K, typename V>
		Iterable<std::pair<K,V>,SDict>::Iter::~Iter()
	{
		if(this->validIter()) {
			flush();
		}
	}
	template<typename K, typename V>
		Iterable<std::pair<K,V>,SDict>::Iterable(TCtnr& ctnr):
			mPCtnr{&ctnr}
	{
		if(TGetObj<K>::kTypeCode != ctnr.mKeyCode) {
			throw BadIterType{
				"Iterable key type does not map to SDict key code"
			};
		}
		if(TGetObj<V>::kTypeCode != ctnr.mValCode) {
			throw BadIterType{
				"Iterable value type does not map to SDict value code"
			};
		}
	}
	template<typename K, typename V>
		auto Iterable<std::pair<K,V>,SDict>::begin() -> Iter
	{
		return Iter{*mPCtnr, mPCtnr->value().begin()};
	}
	template<typename K, typename V>
		auto Iterable<std::pair<K,V>,SDict>::end() -> Iter
	{
		return Iter{*mPCtnr, mPCtnr->value().end()};
	}

	//---- ConstIterable<T,SDict> ---------------------------------------------

	template<typename K, typename V>
		ConstIterable<std::pair<K,V>,SDict>::Iter::Iter(
			const TCtnr& ctnr, TStdIter&& stdIter
		):
		ConstIterBase<TValue,TCtnr>{ctnr, std::forward<TStdIter>(stdIter)}
	{
		if(this->validIter()) {
			load();
		}
	}
	template<typename K, typename V>
		auto ConstIterable<std::pair<K,V>,SDict>::Iter::operator++ ()
		-> Iter&
	{
		return this->increment(), *this;
	}
	template<typename K, typename V>
		auto ConstIterable<std::pair<K,V>,SDict>::Iter::operator++ (int)
		-> Iter
	{
		auto copy = *this;
		return this->increment(), copy;
	}
	template<typename K, typename V>
		void ConstIterable<std::pair<K,V>,SDict>::Iter::load()
	{
		this->mTemp = std::make_pair(
			GetObjVal<K>(this->mStdIter->first),
			GetObjVal<V>(this->mStdIter->second)
		);
	}
	template<typename K, typename V>
		ConstIterable<std::pair<K,V>,SDict>::ConstIterable(
			const TCtnr& ctnr
		):
		mPCtnr{&ctnr}
	{
		if(TGetObj<K>::kTypeCode != ctnr.mKeyCode) {
			throw BadIterType{
				"ConstIterable key type does not map to SDict key code"
			};
		}
		if(TGetObj<V>::kTypeCode != ctnr.mValCode) {
			throw BadIterType{
				"ConstIterable value type does not map to SDict value code"
			};
		}
	}
	template<typename K, typename V>
		auto ConstIterable<std::pair<K,V>,SDict>::begin() const -> Iter
	{
		return Iter{*mPCtnr, mPCtnr->value().cbegin()};
	}
	template<typename K, typename V>
		auto ConstIterable<std::pair<K,V>,SDict>::end() const -> Iter
	{
		return Iter{*mPCtnr, mPCtnr->value().cend()};
	}

}
