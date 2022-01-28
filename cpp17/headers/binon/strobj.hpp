#ifndef BINON_STROBJ_HPP
#define BINON_STROBJ_HPP

#include "mixins.hpp"
#include "hystr.hpp"

namespace binon {

	struct StrObj:
		StdAcc<StrObj>,
		StdEq<StrObj>,
		StdHash<StrObj>,
		StdCodec<StrObj>
	{
		using TValue = HyStr;
		static constexpr auto kTypeCode = kStrObjCode;
		static constexpr auto kClsName = std::string_view{"StrObj"};
		TValue mValue;
		StrObj(TValue v);
		StrObj() = default;
		auto hasDefVal() const noexcept -> bool;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const StrObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> StrObj&;
		void printArgs(std::ostream& stream) const;
	};
}

#endif
