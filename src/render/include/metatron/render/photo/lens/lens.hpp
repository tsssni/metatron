#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::photo {
    namespace lens {
        struct Interaction final {
            math::Ray r;
            f32 pdf;
        };
    }

    MTT_POLY_METHOD(lens_sample, sample);

    struct Lens final: pro::facade_builder
    ::add_convention<lens_sample, auto (
        cref<fv2> o, cref<fv2> u
    ) const noexcept -> opt<lens::Interaction>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
