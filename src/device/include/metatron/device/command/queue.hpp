#pragma once
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/singleton.hpp>
#include <mutex>

namespace mtt::command {
    struct Queue final: stl::capsule<Queue> {
        enum struct Type {
            render,
            transfer,
        };
        auto constexpr static num_types = u32(Type::transfer) + 1;

        std::atomic<u64> timestamp = 1;
        std::mutex mutex;
        u32 family;

        struct Impl;
        Queue() noexcept;
    };

    struct Queues final: stl::singleton<Queues> {
        std::array<Queue, Queue::num_types> queues;
        ~Queues() noexcept;
    };
}
