#pragma once
#include <metatron/resource/texture/texture.hpp>

namespace mtt::texture {
    struct Checkerboard_Texture final {
        Checkerboard_Texture(
            view<spectra::Spectrum> x,
            view<spectra::Spectrum> y,
            math::Vector<usize, 2> uv_scale
        ) noexcept;

        auto operator()(
            device::Sampler const& sampler,
            Coordinate const& coord,
            spectra::Stochastic_Spectrum const& spec
        ) const noexcept -> spectra::Stochastic_Spectrum;
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 2> const& u
        ) const noexcept -> math::Vector<f32, 2>;
        auto pdf(
            math::Vector<f32, 2> const& uv
        ) const noexcept -> f32;

    private:
        view<spectra::Spectrum> x;
        view<spectra::Spectrum> y;
        math::Vector<usize, 2> uv_scale;

        f32 w_x;
        f32 w_y;
    };
}
