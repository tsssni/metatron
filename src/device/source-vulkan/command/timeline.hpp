#pragma once
#include "context.hpp"
#include <metatron/device/command/timeline.hpp>

namespace mtt::command {
    struct Timeline::Impl final {
        vk::UniqueSemaphore semaphore;
    };
}
