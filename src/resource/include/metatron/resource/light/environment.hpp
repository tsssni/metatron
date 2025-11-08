#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::light {
    struct Environment_Light final {
        proxy<texture::Spectrum_Texture> env_map;

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
