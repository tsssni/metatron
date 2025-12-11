#pragma once
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/resource/muldim/image.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::opaque {
    struct Image final: stl::capsule<Image> {
        enum struct State {
            samplable,
            storable,
        };

        command::Queue::Type type;
        State state;
        u64 timestamp = 0;

        u32 width;
        u32 height;
        u32 mips;
        std::vector<obj<Buffer>> host;

        struct Descriptor final {
            command::Queue::Type type;
            State state;
            mut<muldim::Image> image;
        };

        struct Impl;
        Image(cref<Descriptor> desc) noexcept;
    };
}
