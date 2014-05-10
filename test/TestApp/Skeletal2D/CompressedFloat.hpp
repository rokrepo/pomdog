﻿//
//  Copyright (C) 2013-2014 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#ifndef POMDOG_COMPRESSEDFLOAT_727406C1_ACE9_4924_89A8_6A8BCCD56CCC_HPP
#define POMDOG_COMPRESSEDFLOAT_727406C1_ACE9_4924_89A8_6A8BCCD56CCC_HPP

#if (_MSC_VER > 1000)
#	pragma once
#endif

#include <type_traits>
#include <cstdint>
#include <limits>
#include <Pomdog/Utility/Assert.hpp>

namespace Pomdog {
namespace Details {
namespace Skeletal2D {

template <typename T, T Denominator>
class CompressedFloat {
public:
	static_assert(std::is_integral<T>::value, "");

	T data;
	
	CompressedFloat() = default;
	CompressedFloat(float scalar)
		: data(scalar * Denominator)
	{
		POMDOG_ASSERT(scalar < Max());
		POMDOG_ASSERT(scalar >= Min());
	}

	CompressedFloat & operator=(float scalar)
	{
		POMDOG_ASSERT(scalar < Max());
		POMDOG_ASSERT(scalar >= Min());
		data = scalar * Denominator;
		return *this;
	}

	float ToFloat() const
	{
		static_assert(Denominator != 0, "");
		return data * (1.0f / Denominator);
	}
	
	constexpr static float Max()
	{
		static_assert(Denominator != 0, "");
		return std::numeric_limits<T>::max()/Denominator;
	}
	
	constexpr static float Min()
	{
		static_assert(Denominator != 0, "");
		return std::numeric_limits<T>::min()/Denominator;
	}
	
	bool operator<(CompressedFloat const& other) const
	{
		return data < other.data;
	}
};

}// namespace Skeletal2D
}// namespace Details
}// namespace Pomdog

#endif // !defined(POMDOG_COMPRESSEDFLOAT_727406C1_ACE9_4924_89A8_6A8BCCD56CCC_HPP)