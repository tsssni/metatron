#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::light {
    struct Spot_Light final {
        struct Descriptor final {
            stl::proxy<spectra::Spectrum> L;
            f32 falloff_start_theta;
            f32 falloff_end_theta;
        };
        Spot_Light(Descriptor const& desc) noexcept;

        auto operator()(
            math::Ray const& r,
            spectra::Stochastic_Spectrum const& spec
        ) const noexcept -> std::optional<Interaction>;
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 2> const& u
        ) const noexcept -> std::optional<Interaction>;
        auto pdf(
            math::Ray const& r,
            math::Vector<f32, 3> const& np
        ) const noexcept -> f32;
        auto flags() const noexcept -> Flags;

    private:
        stl::proxy<spectra::Spectrum> L;
        f32 falloff_start_cos_theta;
        f32 falloff_end_cos_theta;
    };
}
