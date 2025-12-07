#pragma once
#include <metatron/device/command/queue.hpp>
#include <metatron/device/opaque/buffer.hpp>

namespace mtt::command {
    struct Retention final: stl::capsule<Retention> {
        auto static constexpr num_recorder = 4;
        std::array<std::vector<obj<opaque::Buffer>>, num_recorder> blocks;
        std::array<std::vector<obj<opaque::Buffer>>, num_recorder> update_buffers;
        std::array<std::vector<obj<opaque::Buffer>>, num_recorder> stage_buffers;
        std::array<u64, num_recorder> last_timestamp{0};

        struct Impl;
        Retention() noexcept;
    };

    struct Retentions final: stl::singleton<Retentions> {
        std::array<Retention, Queue::num_types> retentions;
        ~Retentions() noexcept;
    };

    struct Recorder final: stl::capsule<Recorder> {
        Queue::Type type;
        u64 timestamp;
        uptr next = 0;

        struct Impl;
        Recorder(Queue::Type type) noexcept;
        ~Recorder() noexcept;

        auto static queue(u32 family) noexcept -> ref<Queue>;
        auto bind(mut<opaque::Buffer> buffer) noexcept -> void;

    private:
        std::vector<view<opaque::Buffer>> update_dsts;
        std::vector<view<opaque::Buffer>> stage_dsts;
        std::array<std::vector<view<opaque::Buffer>>, Queue::num_types> buffer_transfers;
        std::array<u64, Queue::num_types> wait_timestamps{0};
    };
}
