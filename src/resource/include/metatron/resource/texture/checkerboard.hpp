#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::texture {
    struct Checkerboard_Texture final {
        struct Descriptor final {
            stl::proxy<spectra::Spectrum> x;
            stl::proxy<spectra::Spectrum> y;
            math::Vector<usize, 2> uv_scale;
        };
        Checkerboard_Texture(Descriptor const& desc) noexcept;

        auto operator()(
            image::Coordinate const& coord,
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
        stl::proxy<spectra::Spectrum> const x;
        stl::proxy<spectra::Spectrum> const y;
        math::Vector<usize, 2> uv_scale;

        f32 w_x;
        f32 w_y;
    };
}
