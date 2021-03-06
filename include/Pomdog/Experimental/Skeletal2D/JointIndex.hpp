// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include "Pomdog/Utility/Assert.hpp"
#include <cstdint>
#include <limits>
#include <type_traits>

namespace Pomdog::Skeletal2D::Detail {

template <typename T>
class OptionalUnsigned final {
public:
    static_assert(std::is_unsigned<T>::value, "T is unsigned integer type.");
    T Data;

    constexpr OptionalUnsigned() noexcept
        : Data(std::numeric_limits<T>::max())
    {
    }

    constexpr OptionalUnsigned(T v) noexcept
        : Data(v)
    {
    }

    [[nodiscard]] constexpr bool operator==(const OptionalUnsigned& v) const noexcept
    {
        POMDOG_ASSERT(Data != std::numeric_limits<T>::max());
        return Data == v.Data;
    }

    [[nodiscard]] constexpr bool operator!=(const OptionalUnsigned& v) const noexcept
    {
        POMDOG_ASSERT(Data != std::numeric_limits<T>::max());
        return Data != v.Data;
    }

    [[nodiscard]] const T& operator*() const
    {
        POMDOG_ASSERT(Data != std::numeric_limits<T>::max());
        return Data;
    }

    [[nodiscard]] T& operator*()
    {
        POMDOG_ASSERT(Data != std::numeric_limits<T>::max());
        return Data;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return Data != std::numeric_limits<T>::max();
    }
};

} // namespace Pomdog::Skeletal2D::Detail

namespace Pomdog::Skeletal2D {
using JointIndex = Detail::OptionalUnsigned<std::uint8_t>;
} // namespace Pomdog::Skeletal2D
