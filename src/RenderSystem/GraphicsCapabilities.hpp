// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include <cstddef>

namespace Pomdog::Detail {

struct GraphicsCapabilities final {
    std::size_t ConstantBufferSlotCount;
    std::size_t SamplerSlotCount;
};

} // namespace Pomdog::Detail
