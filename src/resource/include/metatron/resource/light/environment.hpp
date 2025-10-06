#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/image/image.hpp>

namespace mtt::light {
    struct Environment_Light final {
        Environment_Light(
            view<texture::Spectrum_Texture> env_map,
            view<texture::Sampler> sampler
        ) noexcept;

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
        view<texture::Spectrum_Texture> env_map;
        view<texture::Sampler> sampler;
    };
}
