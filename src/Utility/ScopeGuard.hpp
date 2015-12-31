// Copyright (c) 2013-2016 mogemimi. Distributed under the MIT license.

#pragma once

#include "Pomdog/Utility/Assert.hpp"
#include <functional>
#include <utility>

namespace Pomdog {
namespace Detail {

class ScopeGuard final {
public:
    ScopeGuard() = delete;
    ScopeGuard(ScopeGuard const&) = delete;
    ScopeGuard(ScopeGuard &&) = default;

    explicit ScopeGuard(std::function<void()> const& f)
        : func(f)
    {}

    explicit ScopeGuard(std::function<void()> && f)
        : func(std::move(f))
    {}

    ~ScopeGuard()
    {
        POMDOG_ASSERT(func);
        func();
    }

    ScopeGuard & operator=(ScopeGuard const&) = delete;
    ScopeGuard & operator=(ScopeGuard &&) = default;

private:
    std::function<void()> func;
};

} // namespace Detail
} // namespace Pomdog
