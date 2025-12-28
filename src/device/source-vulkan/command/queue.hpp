#pragma once
#include "context.hpp"
#include <metatron/device/command/queue.hpp>

namespace mtt::command {
    struct Family final {
        u32 idx = math::maxv<u32>;
        u32 flags = 0;
        u32 start = 0;
        u32 count = 0;
        u32 allocated = 0;
    };

    struct Queue::Impl final {
        std::array<Family, command::num_types> static families;
        std::atomic_flag flag = false;
        vk::Queue queue;
        std::vector<vk::UniqueCommandPool> pools;
        std::vector<std::deque<obj<Buffer>>> cmds;
    };
}
