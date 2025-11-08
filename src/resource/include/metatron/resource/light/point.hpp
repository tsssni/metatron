#pragma once
#include <metatron/resource/light/light.hpp>

namespace mtt::light {
    struct Point_Light final {
        proxy<spectra::Spectrum> L;

        auto operator()(
            math::Ray const& r,
            spectra::Stochastic_Spectrum const& spec
        ) const noexcept -> std::optional<Interaction>;
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 2> const& u
        ) const noexcept -> std::optional<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
