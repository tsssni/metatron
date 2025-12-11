#pragma once
#include "context.hpp"
#include <metatron/device/command/queue.hpp>

namespace mtt::command {
    struct Queue::Impl final {
        vk::Queue queue;
        vk::UniqueSemaphore timeline;
    };
}
