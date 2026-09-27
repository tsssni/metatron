#pragma once
#include <metatron/render/filter/interaction.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>

namespace mtt::filter {
    struct Lanczos_Filter final {
        struct Descriptor final {
            fv2 radius = {0.5f};
            f32 tau = 3.f;
        };
        Lanczos_Filter(Descriptor const& desc) noexcept;
        Lanczos_Filter() noexcept = default;
        auto operator()(fv2 const& p) const noexcept -> f32;
        auto sample(fv2 const& u) const noexcept -> Interaction;

    private:
        math::proxy::Planar_Distribution distr;
        fv2 radius;
        f32 tau;
    };
}
