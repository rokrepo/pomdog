// Copyright (c) 2013-2019 mogemimi. Distributed under the MIT license.

#pragma once

#include <Pomdog/Pomdog.hpp>

namespace Pomdog {

class GameHost;

class GameLevel {
public:
    GameLevel() = default;
    GameLevel(const GameLevel&) = delete;
    GameLevel& operator=(const GameLevel&) = delete;

    virtual ~GameLevel() = default;

    virtual void Update(GameHost& gameHost, EntityManager& manager) = 0;
};

} // namespace Pomdog
