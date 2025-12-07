#pragma once
#include "context.hpp"
#include <metatron/device/command/recorder.hpp>

namespace mtt::command {
    struct Retention::Impl final {
        vk::UniqueCommandPool pool;
        std::array<vk::UniqueCommandBuffer, num_recorder> buffers;
    };

    struct Recorder::Impl final {
        vk::Queue queue;
        vk::Semaphore timeline;
        vk::CommandPool pool;
        vk::CommandBuffer buffer;

        u32 family;
        u64 timestamp;

        auto static init() noexcept -> void;
    };
}
