#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::device {
    struct Sampler final {
        enum struct Filter: usize {
            none,
            nearest,
            linear,
        };

        enum struct Wrap: usize {
            repeat,
            mirror,
            edge,
            border,
        };

        Filter min_filter{Filter::linear};
        Filter mag_filter{Filter::linear};
        Filter mip_filter{Filter::linear};

        Wrap wrap_u{Wrap::repeat};
        Wrap wrap_v{Wrap::repeat};
        Wrap wrap_w{Wrap::repeat};

        float anisotropy{16.f};
        float lod_bias{0.f};
        float min_lod{0.f};
        float max_lod{1024.f};

        math::Vector<f32, 4> border{0.f};
    };
}
