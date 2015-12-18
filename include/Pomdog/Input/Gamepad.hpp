// Copyright (c) 2013-2015 mogemimi. Distributed under the MIT license.

#pragma once

#include "Pomdog/Basic/Export.hpp"

namespace Pomdog {

class GamepadState;
class GamepadCapabilities;

class POMDOG_EXPORT Gamepad {
public:
    virtual ~Gamepad() = default;

    virtual GamepadCapabilities GetCapabilities() const = 0;

    virtual GamepadState GetState() const = 0;
};

} // namespace Pomdog
