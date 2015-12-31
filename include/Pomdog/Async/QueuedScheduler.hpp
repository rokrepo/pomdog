// Copyright (c) 2013-2016 mogemimi. Distributed under the MIT license.

#pragma once

#include "Pomdog/Async/Scheduler.hpp"
#include <vector>
#include <mutex>

namespace Pomdog {
namespace Concurrency {

class POMDOG_EXPORT QueuedScheduler final : public Scheduler {
private:
    typedef std::chrono::steady_clock clockType;
    typedef clockType::time_point TimePoint;

    struct DeferredTask {
        std::function<void()> Function;
        TimePoint StartTime;

        bool operator<(const DeferredTask& other) const noexcept
        {
            return this->StartTime < other.StartTime;
        }
    };

    std::vector<DeferredTask> tasks;
    std::vector<DeferredTask> addedDeferredTasks;
    std::recursive_mutex addingProtection;
    std::recursive_mutex tasksProtection;

public:
    void Schedule(
        std::function<void()> && task,
        const Duration& delayTime = Duration::zero()) override;

    void Update();

    bool Empty() noexcept;

private:
    void MergeTasks();
};

} // namespace Concurrency
} // namespace Pomdog
