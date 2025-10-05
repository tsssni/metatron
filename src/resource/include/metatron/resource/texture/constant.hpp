#pragma once
#include <metatron/resource/texture/texture.hpp>

namespace mtt::texture {
    struct Constant_Spectrum_Texture final {
        Constant_Spectrum_Texture(view<spectra::Spectrum> x) noexcept;

        auto operator()(
            Sampler const& sampler,
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
    };

    struct Constant_Vector_Texture final {
        Constant_Vector_Texture(math::Vector<f32, 4> const& x) noexcept;

        auto operator()(
            Sampler const& sampler,
            Coordinate const& coord
        ) const noexcept -> math::Vector<f32, 4>;
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 2> const& u
        ) const noexcept -> math::Vector<f32, 2>;
        auto pdf(
            math::Vector<f32, 2> const& uv
        ) const noexcept -> f32;

    private:
        math::Vector<f32, 4> x;
    };
}
