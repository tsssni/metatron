#pragma once
#include <metatron/device/command/block.hpp>
#include <metatron/device/command/timeline.hpp>

namespace mtt::command {
    struct Buffer final: stl::capsule<Buffer> {
        Queue::Type type;
        Blocks blocks;
        std::vector<obj<opaque::Buffer>> stages;
        using Timeline_Count = std::tuple<mut<Timeline>, u64>;
        std::vector<Timeline_Count> waits;
        std::vector<Timeline_Count> signals;

        struct Impl;
        Buffer() noexcept;
        ~Buffer() noexcept;
    };
}
