// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include "Pomdog/Graphics/InputElement.hpp"
#include <vector>

namespace Pomdog {

struct InputLayoutDescription final {
    std::vector<Pomdog::InputElement> InputElements;
};

} // namespace Pomdog
