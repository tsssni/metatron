#pragma once
#include <metatron/device/command/queue.hpp>
#include <metatron/resource/muldim/image.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::opaque {
    struct Image final: stl::capsule<Image> {
        command::Queue::Type type;
        u64 timestamp = 0;

        u32 width;
        u32 height;
        u32 mips;
        std::vector<std::vector<byte>> host;

        struct Impl;
        Image(muldim::Image&& image) noexcept;
    };
}
