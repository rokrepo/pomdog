// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include <cstdint>

namespace Pomdog {

enum class TextureAddressMode : std::uint8_t {
    Wrap,
    Clamp,
    Mirror,
    Border,
};

} // namespace Pomdog
