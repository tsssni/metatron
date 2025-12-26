#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::command {
    struct Buffer;
}

namespace mtt::opaque {
    struct Sampler final: stl::capsule<Sampler> {
        enum struct Mode {
            repeat,
            mirror,
            clamp,
            border,
        } mode;
        enum struct Border {
            transparent,
            opaque,
            white,
        } border;

        struct Descriptor final {
            Mode mode = Mode::repeat;
            Border border = Border::transparent;
        };

        struct Impl;
        Sampler(cref<Descriptor> desc) noexcept;
    };
}
