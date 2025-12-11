#pragma once
#include <metatron/device/command/queue.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/image.hpp>
#include <metatron/device/opaque/grid.hpp>

namespace mtt::command {
    struct Retention final: stl::capsule<Retention> {
        auto constexpr static num_recorder = 4;
        auto constexpr static block_size = 1 << 26;

        using Buffer_List = std::vector<obj<opaque::Buffer>>;
        using Mip_List = std::vector<Buffer_List>;
        template<typename T>
        using Recorder_List = std::array<T, num_recorder>;

        Recorder_List<Buffer_List> blocks;
        Recorder_List<Buffer_List> update_buffers;
        Recorder_List<Buffer_List> stage_buffers;
        Recorder_List<Mip_List> image_buffers;
        Recorder_List<Buffer_List> grid_buffers;
        Recorder_List<u64> last_timestamp{0};

        struct Impl;
        Retention() noexcept;
    };

    struct Retentions final: stl::singleton<Retentions> {
        std::array<Retention, Queue::num_types> retentions;
        ~Retentions() noexcept;
    };

    struct Buffer final: stl::capsule<Buffer> {
        Queue::Type type;
        u64 timestamp;
        uptr next = 0;

        struct Impl;
        Buffer(Queue::Type type) noexcept;
        ~Buffer() noexcept;

        auto bind(mut<opaque::Buffer> buffer) noexcept -> void;
        auto bind(mut<opaque::Image> image) noexcept -> void;
        auto bind(mut<opaque::Grid> grid) noexcept -> void;

    private:
        auto allocate(usize size) noexcept -> uzv2;

        template<typename T>
        using Destination_List = std::vector<mut<T>>;
        template<typename T>
        using Queue_List = std::array<T, Queue::num_types>;

        using Buffer_List = Destination_List<opaque::Buffer>;
        using Image_List = Destination_List<opaque::Image>;
        using Grid_List = Destination_List<opaque::Grid>;

        Buffer_List update_dsts;
        Buffer_List stage_dsts;
        Image_List image_dsts;
        Grid_List grid_dsts;

        Queue_List<Buffer_List> buffer_transfers;
        Queue_List<Image_List> image_transfers;
        Queue_List<Grid_List> grid_transfers;

        Queue_List<u64> wait_timestamps{0};
    };
}
