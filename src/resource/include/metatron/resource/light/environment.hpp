#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/image/image.hpp>
#include <metatron/core/math/distribution/sphere.hpp>
#include <metatron/core/math/distribution/sphere.hpp>

namespace mtt::light {
    struct Environment_Light final {
        Environment_Light(view<texture::Spectrum_Texture> env_map) noexcept;

        auto operator()(
            eval::Context const& ctx
        ) const noexcept -> std::optional<Interaction>;
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 2> const& u
        ) const noexcept -> std::optional<Interaction>;
        auto flags() const noexcept -> Flags;

    private:
        view<texture::Spectrum_Texture> env_map;
    };
}
