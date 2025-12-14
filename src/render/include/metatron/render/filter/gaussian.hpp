#pragma once
#include <metatron/render/filter/filter.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>

namespace mtt::filter {
    struct Gaussian_Filter final {
        struct Descriptor final {
            fv2 radius = {1.5f};
            f32 sigma = 0.5f;
        };
        Gaussian_Filter() noexcept = default;
        Gaussian_Filter(cref<Descriptor> desc) noexcept;
        auto operator()(cref<fv2> p) const noexcept -> f32;
        auto sample(cref<fv2> u) const noexcept -> opt<Interaction>;

    private:
        math::Planar_Distribution piecewise;
        fv2 radius;
        f32 sigma;
    };
}
