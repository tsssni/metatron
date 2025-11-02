#pragma once
#include <metatron/resource/phase/phase-function.hpp>

namespace mtt::phase {
    struct Henyey_Greenstein_Phase_Function final {
        f32 g;
        spectra::Stochastic_Spectrum spectrum;

        auto operator()(
            math::Vector<f32, 3> const& wo,
            math::Vector<f32, 3> const& wi
        ) const noexcept -> std::optional<Interaction>;
        auto sample(eval::Context const& ctx, math::Vector<f32, 2> const& u) const noexcept -> std::optional<Interaction>;
    };
}
