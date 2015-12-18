// Copyright (c) 2013-2015 mogemimi. Distributed under the MIT license.

#pragma once

#include "InputClassification.hpp"
#include "InputElementFormat.hpp"
#include <cstdint>

namespace Pomdog {

struct InputElement {
    std::uint16_t ByteOffset;
    std::uint16_t InputSlot;
    InputElementFormat Format;
    InputClassification InputSlotClass;
    std::uint16_t InstanceStepRate;
};

} // namespace Pomdog
