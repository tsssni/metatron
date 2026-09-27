#pragma once
#include "context.hpp"
#include <metatron/device/command/queue.hpp>

namespace mtt::command {
    struct Queue::Impl final {
        mtl<MTL::CommandQueue> queue;
        std::vector<std::deque<std::unique_ptr<Buffer>>> cmds;
    };
}
