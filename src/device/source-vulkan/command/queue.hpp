#pragma once
#include "context.hpp"
#include <metatron/device/command/queue.hpp>

namespace mtt::command {
    struct Queue::Impl final {
        std::array<u32, command::num_types> static family;
        std::array<u32, command::num_types> static count;
        std::array<u32, command::num_types> static idx;
        std::atomic_flag flag = false;
        vk::Queue queue;
        std::vector<vk::UniqueCommandPool> pools;
        std::vector<std::deque<obj<Buffer>>> cmds;
    };
}
