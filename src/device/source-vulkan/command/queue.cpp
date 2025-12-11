#include "queue.hpp"

namespace mtt::command {
    Queue::Queue() noexcept {}

    Queues::~Queues() noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        for (auto i = 0; i < Queue::num_types; ++i)
            guard(queues[i].impl->queue.waitIdle());
    }
}
