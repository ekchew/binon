#ifndef BINON_NULLOBJ_HPP
#define BINON_NULLOBJ_HPP

#include "mixins.hpp"
#include <cstddef>

namespace binon {
	struct NullObj: StdCodec<NullObj> {
		using TValue = std::nullptr_t;
		TValue mValue = nullptr;
		static constexpr auto kTypeCode = kNullObjCode;
		static constexpr auto kClsName = std::string_view{"NullObj"};
		auto value() const { return mValue; }
		auto& value() { return mValue; }
		constexpr auto operator== (NullObj) const noexcept { return true; }
		constexpr auto operator!= (NullObj) const noexcept { return false; }
		auto hash() const noexcept -> std::size_t {
				return std::hash<CodeByte>{}(kTypeCode);
			}
		constexpr auto hasDefVal() const noexcept { return false; }
		constexpr void encodeData(TOStream&, bool requireIO = true)
			const noexcept {}
		constexpr void decodeData(TIStream&, bool requireIO = true)
			noexcept {}
		constexpr void printArgs(std::ostream&) const noexcept {}
	};
}

#endif
