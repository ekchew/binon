#ifndef BINON_STROBJ_HPP
#define BINON_STROBJ_HPP

#include "mixins.hpp"
#include "hystr.hpp"

namespace binon {

	struct TStrObj:
		TStdAcc<TStrObj>,
		TStdEq<TStrObj>,
		TStdHash<TStrObj>,
		TStdCodec<TStrObj>
	{
		using TValue = HyStr;
		static constexpr auto kTypeCode = kStrObjCode;
		static constexpr auto kClsName = std::string_view{"TStrObj"};
		TValue mValue;
		TStrObj(TValue v);
		TStrObj() = default;
		auto hasDefVal() const noexcept -> bool;
		auto encodeData(TOStream& stream, bool requireIO = true) const
			-> const TStrObj&;
		auto decodeData(TIStream& stream, bool requireIO = true)
			-> TStrObj&;
		void printArgs(std::ostream& stream) const;
	};
}

#endif
