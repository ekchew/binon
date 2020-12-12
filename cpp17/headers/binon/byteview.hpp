#ifndef BINON_BYTEVIEW_HPP
#define BINON_BYTEVIEW_HPP

//	When reading a bytes from an istream, you generally wind up reading into a
//	char (a.k.a. binon::StreamByte) -- as opposed to a std::byte -- buffer.
//	In theory, though, you should be using std::byte to work with byte
//	quantities. That's what it's there for.
//
//	So this header defines a couple of classes that give you std::byte views of
//	an underlying buffer.

#include "byteutil.hpp"

#include <sstream>
#include <stdexcept>

namespace binon {
	
	//	ByteView gives you an immutable view of an underlying random access
	//	buffer in std::byte form. The value type of this buffer must be of
	//	of byte length (e.g. StreamByte).
	template<T> class ByteView {
		static_assert(sizeof(T) == 1,
			"binon::ByteView/Span requires a byte-length type"
			);
	
	public:
		using buf_value_type = T;
		using value_type = std::byte;
		using size_type = std::size_t;
		
		class ConstIterator {
		public:
			constexpr ConstIterator(const T* pByte) noexcept: mPByte{pByte} {}
			constexpr auto operator*() const noexcept {return ToByte(*mPByte);}
			constexpr auto operator==(const ConstIterator& rhs) const noexcept
				{ return mPByte == rhs.mPByte; }
			constexpr auto& operator++() const noexcept
				{ return ++mPByte, *this; }
			constexpr auto& operator++(int) const noexcept
				{ auto copy = *this; return ++mPByte, copy; }
		
		protected:
			//	The byte pointer is made mutable for ByteSpan::Iterator's sake.
			//	ConstIterator itself will treat it as const.
			mutable T* mPByte;
		};
		
		constexpr ByteView(const T* pBuf, size_type size) noexcept:
			mPBuf{buf}, mSize{size} {}
		constexpr size() const noexcept {return mSize;}
		constexpr auto operator[](size_type i) const noexcept
			{ return ToByte(mPBuf[i]); }
		constexpr auto at(size_type i) const {return checkI(i), (*this)[i];}
		constexpr auto cbegin() const noexcept
			{ return ConstIterator{mPBuf}; }
		constexpr auto cend() const noexcept
			{ return ConstIterator{mPBuf + mSize}; }
		constexpr auto begin() const noexcept {return cbegin();}
		constexpr auto end() const noexcept {return cend();}
	
	protected:
		
		//	The buffer pointer is made mutable for ByteSpan's sake.
		//	ByteView itself will treat it as const.
		mutable T* mPBuf;
		size_type mSize;
		
		constexpr void checkI(size_type i) const {
				if(i >= mSize) {
					std::ostringstream oss;
					oss << "expected binon::ByteView/Span index < "
						<< mSize << " but got " << i;
					throw std::out_of_range{oss.str()};
				}
			}
	};
	
	//	A ByteSpan is a mutable version of a ByteView. Note that when you
	//	dereference individual bytes, ByteSpan returns a ByteRef instance
	//	rather than a regular C++ & reference. Do not, therefore, try to save
	//	this reference to an auto& variable. You can go with auto&& or simply
	//	auto instead. (Personally, I like auto&& because it works on both
	//	regular and proxy class references like this.)
	template<T> class ByteSpan: public ByteView<T> {
	public:
		class ByteRef {
		public:
			constexpr ByteRef(T& v) noexcept: mByte{v} {}
			constexpr operator std::byte() const noexcept
				{ return ToByte(mByte); }
			constexpr auto& operator=(std::byte v) noexcept
				{ return mByte = std::to_integer<T>(v), *this; }
		
		private:
			T& mByte;
		};
		class Iterator: public ConstIterator {
		public:
			using ConstIterator::ConstIterator;
			constexpr auto operator*() noexcept {return ByteRef{*mPByte};}
		};
		
		using ByteView::ByteView;
		constexpr auto operator[](size_type i) noexcept
			{ return ByteRef{mPBuf[i]}; }
		constexpr auto at(size_type i) {return checkI(i), (*this)[i];}
		constexpr auto begin() noexcept {return Iterator{mPBuf};}
		constexpr auto end() noexcept {return Iterator{mPBuf + mSize};}
	};

}

#endif
