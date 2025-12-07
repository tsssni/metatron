#pragma once
#include <metatron/device/command/queue.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/image.hpp>

namespace mtt::command {
    struct Retention final: stl::capsule<Retention> {
        auto constexpr static num_recorder = 4;
        auto constexpr static block_size = 1 << 26;
        std::array<std::vector<obj<opaque::Buffer>>, num_recorder> blocks;
        std::array<std::vector<obj<opaque::Buffer>>, num_recorder> update_buffers;
        std::array<std::vector<obj<opaque::Buffer>>, num_recorder> stage_buffers;
        std::array<std::vector<std::vector<obj<opaque::Buffer>>>, num_recorder> image_buffers;
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

        auto bind(mut<opaque::Buffer> buffer) noexcept -> void;
        auto bind(mut<opaque::Image> image) noexcept -> void;

    private:
        auto allocate(usize size) noexcept -> uzv2;

        std::vector<mut<opaque::Buffer>> update_dsts;
        std::vector<mut<opaque::Buffer>> stage_dsts;
        std::vector<mut<opaque::Image>> image_dsts;
        std::array<std::vector<mut<opaque::Buffer>>, Queue::num_types> buffer_transfers;
        std::array<std::vector<mut<opaque::Image>>, Queue::num_types> image_transfers;
        std::array<u64, Queue::num_types> wait_timestamps{0};
    };
}
