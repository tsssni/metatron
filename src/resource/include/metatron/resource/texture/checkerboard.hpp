#pragma once
#include <metatron/resource/texture/texture.hpp>

namespace mtt::texture {
    struct Checkerboard_Texture final {
        Checkerboard_Texture(
            view<spectra::Spectrum> x,
            view<spectra::Spectrum> y,
            math::Vector<usize, 2> uv_scale
        ) noexcept;

        auto sample(
            eval::Context const& ctx,
            Sampler const& sampler,
            Coordinate const& coord
        ) const noexcept -> spectra::Stochastic_Spectrum;

    private:
        view<spectra::Spectrum> x;
        view<spectra::Spectrum> y;
        math::Vector<usize, 2> uv_scale;
    };
}
