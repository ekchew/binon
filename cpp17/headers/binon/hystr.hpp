#ifndef BINON_HYSTR_HPP
#define BINON_HYSTR_HPP

#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace binon {

	/**
	BasicHyStr class template

	This class attempts to get the best of both worlds between strings and
	string views. It can store one or the other, but not both simultaneously.
	(It uses std::variant internally to store the two types.) You can check if
	it is a string (as opposed to a view) by calling isStr().

	BasicHyStr is a hybrid of std::basic_string and std::basic_string_view. But
	in most cases, you will probably want to go with the simpler HyStr type: a
	hybrid of std::string and std::string_view.

	Which type it starts out with depends on whether you constructed it with a
	string or a view. (The default constructor will go with an empty view.) In
	either case, you can get a view of the object by calling asView(). This will
	not modify the object in any way.

	asStr() returns a reference to the stored string if it exists. If a view is
	stored instead, asStr() will replace the view with a string containing the
	same characters and return that. This will likely necessitate some dynamic
	memory allocation and copying.

	Note that any method (with the exception of clear()) that modifies the
	object (e.g. resize(), the non-const version data(), etc.) will implicitly
	call asStr(), thus converting the object from a view to a string if
	necessary.

	BasicHyStr has yet to expose all the APIs of a basic_string. Here are the
	ones available so far:

		at()
		operator []
			This will actually call at() in debug mode.
		data()
		begin/end() (plus c/r/cr variants of these)
			You should get either a string iterator or a string view
			const_iterator (but never a string const_iterator).
		empty()
		size/length()
		clear()
			This is the only non-const method that doesn't call asStr(). If the
			object is a string view, it simply replaces it with an empty view.
		resize()
		operators << and >> (for streaming)
		operators ==, !=, <, >, <=, >=
		std::hash specialization

	You can always call more standard methods on what is returned by asView()
	and asStr(), of course.

	Template Args:
		CharT (required): see std::basic_string
		Traits (required): see std::basic_string
		Allocator (required): see std::basic_string

	Type Definitions:
		TStr: string type
		TView: string view type
		TChr: character type
		TSize: size type for character indices, etc.
		value_type: see std::basic_string
		traits_type: see std::basic_string
		allocator_type: see std::basic_string
		size_type: see std::basic_string
	**/
	template<
		typename CharT,
		typename Traits = std::char_traits<CharT>,
		typename Allocator = std::allocator<CharT>
		>
		struct BasicHyStr {

			//---- Type Definitions Unique To BasicHyStr -----------------------

			using TStr = std::basic_string<CharT,Traits,Allocator>;
			using TView = std::basic_string_view<CharT,Traits>;
			using TChr = typename TStr::value_type;
			using TSize = typename TStr::size_type;

			//---- Type Definitions Found In std::string -----------------------

			using value_type = TChr;
			using traits_type = typename TStr::traits_type;
			using allocator_type = typename TStr::allocator_type;
			using size_type = TSize;

			//---- Friend Functions --------------------------------------------

			template<typename C, typename T, typename A>
				friend auto operator << (
					std::basic_ostream<C,T>& os,
					const BasicHyStr<C,T,A>& hs) -> std::basic_ostream<C,T>&;
			
			//---- Constructors ------------------------------------------------

			/**
			constructor - string variants

			Loads the BasicHyStr with a string.

			Args:
				s (const TStr& or TStr&&)
			**/
			BasicHyStr(const TStr& s);
			BasicHyStr(TStr&& s) noexcept;

			/**
			constructor - string view variants

			Loads the BasicHyStr with a string view. The default constructor
			will give you an empty string view.

			Args:
				s (const TView& or const char*, optional):
					If you arg is a const char*, BasciHyStr assumes it points to
					a C string.
			**/
			constexpr BasicHyStr(const TView& sv) noexcept;
			constexpr BasicHyStr(const char* cStr) noexcept;
			constexpr BasicHyStr() noexcept = default;

			//---- Methods Found In std::string --------------------------------
			//
			//	These are not specially documented aside from some class notes
			//	where appropriate, since they are already described in the
			//	standard library documentation.

			auto at(TSize i) -> TChr&;
			constexpr auto at(TSize i) const -> TChr;
			auto operator [] (TSize i) -> TChr&;
			constexpr auto operator [] (TSize i) const -> TChr;
			auto operator += (const BasicHyStr& hs) -> BasicHyStr&;
			auto data() -> TChr*;
			constexpr auto data() const noexcept -> const TChr*;
			auto begin() { return asStr().begin(); }
			constexpr auto begin() const noexcept { return asView().begin(); }
			constexpr auto cbegin() const noexcept { return asView().cbegin(); }
			auto end() { return asStr().end(); }
			constexpr auto end() const noexcept { return asView().end(); }
			constexpr auto cend() const noexcept { return asView().cend(); }
			auto rbegin() { return asStr().rbegin(); }
			constexpr auto rbegin() const noexcept { return asView().rbegin(); }
			constexpr auto crbegin() const noexcept {
					return asView().crbegin();
				}
			auto rend() { return asStr().rend(); }
			constexpr auto rend() const noexcept { return asView().rend(); }
			constexpr auto crend() const noexcept { return asView().crend(); }
			void clear() noexcept;
			constexpr auto empty() const noexcept -> bool;
			constexpr auto size() const noexcept -> TSize;
			constexpr auto length() const noexcept -> TSize;
			void resize(TSize n);
			void resize(TSize n, TChr c);

			//---- Methods Unique To BasicHyStr --------------------------------

			/**
			isStr method:
				Returns:
					bool: true if object contains a string (vs. a string_view)
			**/
			constexpr auto isStr() const noexcept -> bool;
			
			/**
			asView method, operator TView:
				An implicit conversion from BasicHyStr to TView is available
				when needed.

				Returns:
					TView: string view of current object returned by value
			**/
			constexpr auto asView() const noexcept -> TView;
			constexpr operator TView() const noexcept;
			
			/**
			asStr method:
				This method will convert the internal string view into a string
				if necessary before returning it.

				Returns:
					TStr&: reference to internal string
			**/
			auto asStr() -> TStr&;
			
			/**
			hash method:
				This method is called by a specialization of std::hash for
				BasicHyStr types.

				Returns:
					std::size_t: a hash of the internal string/view
			**/
			constexpr auto hash() const noexcept -> std::size_t;

		 private:
			std::variant<TView,TStr> mV;
		};

	//---- Free Functions Relating to BasicHyStr -------------------------------

	template<typename C, typename T, typename A>
		constexpr auto operator == (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool;
	template<typename C, typename T, typename A>
		constexpr auto operator != (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool;
	template<typename C, typename T, typename A>
		constexpr auto operator < (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool;
	template<typename C, typename T, typename A>
		constexpr auto operator > (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool;
	template<typename C, typename T, typename A>
		constexpr auto operator <= (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool;
	template<typename C, typename T, typename A>
		constexpr auto operator >= (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool;
	template<typename C, typename T, typename A>
		auto operator << (std::basic_ostream<C,T>& os,
			const BasicHyStr<C,T,A>& hs) -> std::basic_ostream<C,T>&;
	template<typename C, typename T, typename A>
		auto operator >> (std::basic_istream<C,T>& is, BasicHyStr<C,T,A>& hs);

	//---- BasicHyStr Specializations ------------------------------------------

	using HyStr = BasicHyStr<std::string::value_type>;

	//==== Template Implementation =============================================

	template<typename C, typename T, typename A>
		BasicHyStr<C,T,A>::BasicHyStr(const TStr& s):
			mV{s}
		{
		}
	template<typename C, typename T, typename A>
		BasicHyStr<C,T,A>::BasicHyStr(TStr&& s) noexcept:
			mV{std::move(s)}
		{
		}
	template<typename C, typename T, typename A>
		constexpr BasicHyStr<C,T,A>::BasicHyStr(const TView& sv) noexcept:
			mV{sv}
		{
		}
	template<typename C, typename T, typename A>
		constexpr BasicHyStr<C,T,A>::BasicHyStr(const char* cStr) noexcept:
			mV{static_cast<TView>(cStr)}
		{
		}
	template<typename C, typename T, typename A>
		auto BasicHyStr<C,T,A>::at(TSize i) -> TChr& {
			auto& s = asStr();
			return s.at(i);
		}
	template<typename C, typename T, typename A>
		constexpr auto BasicHyStr<C,T,A>::at(TSize i) const -> TChr {
			return std::visit(
				[i](const auto& v)->TChr { return v.at(i); }, mV);
		}
	template<typename C, typename T, typename A>
		auto BasicHyStr<C,T,A>::operator [] (TSize i) -> TChr& {
			auto& s = asStr();
			return BINON_IF_DBG_REL(s.at(i), s[i]);
		}
	template<typename C, typename T, typename A>
		constexpr auto BasicHyStr<C,T,A>::operator [] (TSize i) const -> TChr {
			return std::visit(
				[i](const auto& v)->TChr {
					return BINON_IF_DBG_REL(v.at(i), v[i]);
				}, mV);
		}
	template<typename C, typename T, typename A>
		auto BasicHyStr<C,T,A>::operator += (const BasicHyStr& hs)
			-> BasicHyStr&
		{
			std::visit([this](const auto& v) { asStr() += v; }, hs.mV);
			return *this;
		}
	template<typename C, typename T, typename A>
		auto BasicHyStr<C,T,A>::data() -> TChr* {
			return asStr().data();
		}
	template<typename C, typename T, typename A>
		constexpr auto BasicHyStr<C,T,A>::data() const noexcept -> const TChr* {
			return std::visit(
				[](const auto& v)->const TChr* { return v.data(); }
				, mV);
		}
	template<typename C, typename T, typename A>
		void BasicHyStr<C,T,A>::clear() noexcept {
			if(isStr()) {
				std::get<1>(mV).clear();
			}
			else {
				mV = TView{};
			}
		}
	template<typename C, typename T, typename A>
		constexpr auto BasicHyStr<C,T,A>::empty() const noexcept -> bool {
			return std::visit(
				[](const auto& v)->bool { return v.empty(); }, mV);
		}
	template<typename C, typename T, typename A>
		constexpr auto BasicHyStr<C,T,A>::size() const noexcept -> TSize {
			return std::visit(
				[](const auto& v)->TSize { return v.size(); }, mV);
		}
	template<typename C, typename T, typename A>
		constexpr auto BasicHyStr<C,T,A>::length() const noexcept -> TSize {
			return size();
		}
	template<typename C, typename T, typename A>
		void BasicHyStr<C,T,A>::resize(TSize n) {
			asStr().resize(n);
		}
	template<typename C, typename T, typename A>
		void BasicHyStr<C,T,A>::resize(TSize n, TChr c) {
			asStr().resize(n, c);
		}
	template<typename C, typename T, typename A>
		constexpr auto BasicHyStr<C,T,A>::isStr() const noexcept -> bool {
			return mV.index() != 0;
		}
	template<typename C, typename T, typename A>
		constexpr auto BasicHyStr<C,T,A>::asView() const noexcept -> TView {
			return std::visit(
				[](const auto& v)->TView { return v; }, mV);
		}
	template<typename C, typename T, typename A>
		auto BasicHyStr<C,T,A>::asStr() -> TStr& {
			if(!isStr()) {
				auto& sv = std::get<0>(mV);
				mV = TStr(sv.begin(), sv.end());
			}
			return std::get<1>(mV);
		}
	template<typename C, typename T, typename A>
		constexpr BasicHyStr<C,T,A>::operator TView() const noexcept {
			return asView();
		}
	template<typename C, typename T, typename A>
		constexpr auto BasicHyStr<C,T,A>::hash() const noexcept -> std::size_t {
			return std::visit(
				[](const auto& v)->std::size_t {
					using TV = std::decay_t<decltype(v)>;
					return std::hash<TV>{}(v);
				}, mV);
		}
	template<typename C, typename T, typename A>
		constexpr auto operator == (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool
		{
			return a.asView() == b.asView();
		}
	template<typename C, typename T, typename A>
		constexpr auto operator != (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool
		{
			return a.asView() != b.asView();
		}
	template<typename C, typename T, typename A>
		constexpr auto operator < (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool
		{
			return a.asView() < b.asView();
		}
	template<typename C, typename T, typename A>
		constexpr auto operator > (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool
		{
			return a.asView() > b.asView();
		}
	template<typename C, typename T, typename A>
		constexpr auto operator <= (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool
		{
			return a.asView() <= b.asView();
		}
	template<typename C, typename T, typename A>
		constexpr auto operator >= (
			const BasicHyStr<C,T,A>& a,
			const BasicHyStr<C,T,A>& b) noexcept -> bool
		{
			return a.asView() >= b.asView();
		}
	template<typename C, typename T, typename A>
		auto operator << (std::basic_ostream<C,T>& os,
			const BasicHyStr<C,T,A>& hs) -> std::basic_ostream<C,T>&
		{
			std::visit([&os](const auto& v) { os << v; }, hs.mV);
			return os;
		}
	template<typename C, typename T, typename A>
		auto operator >> (std::basic_istream<C,T>& is, BasicHyStr<C,T,A>& hs)
			-> std::basic_istream<C,T>&
		{
			return is >> hs.asStr();
		}
}

namespace std {
	template<typename C, typename T, typename A>
		struct hash<binon::BasicHyStr<C,T,A>> {
			constexpr auto operator () (const binon::BasicHyStr<C,T,A>& v)
				const noexcept -> std::size_t { return v.hash(); }
		};
}

#endif
