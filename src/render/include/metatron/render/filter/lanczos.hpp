#pragma once
#include <metatron/render/filter/filter.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>

namespace mtt::filter {
    struct Lanczos_Filter final {
        struct Descriptor final {
            fv2 radius = {0.5f};
            f32 tau = 3.f;
        };
        Lanczos_Filter() noexcept = default;
        Lanczos_Filter(cref<Descriptor> desc) noexcept;
        auto operator()(cref<fv2> p) const noexcept -> f32;
        auto sample(cref<fv2> u) const noexcept -> opt<Interaction>;

    private:
        math::Planar_Distribution distribution;
        fv2 radius;
        f32 tau;
    };
}
