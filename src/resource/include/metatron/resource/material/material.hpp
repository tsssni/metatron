#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/resource/media/medium.hpp>
#include <unordered_map>
#include <functional>

namespace mtt::material {
    struct Interaction final {
        poly<bsdf::Bsdf> bsdf;
        spectra::Stochastic_Spectrum emission;
        math::Vector<f32, 3> normal{0.f};
        bool degraded{false};
    };

    struct Material final {
        std::function<auto (bsdf::Attribute const&) -> poly<bsdf::Bsdf>> configurator;

        std::unordered_map<
            std::string,
            view<texture::Spectrum_Texture>
        > spectrum_textures;

        std::unordered_map<
            std::string,
            view<texture::Vector_Texture>
        > vector_textures;

        std::unordered_map<
            std::string,
            view<device::Sampler>
        > samplers;

        auto sample(
            eval::Context const& ctx,
            texture::Coordinate const& coord
        ) const noexcept -> std::optional<Interaction>;
    };
}
