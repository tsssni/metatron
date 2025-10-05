#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/image/image.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>

namespace mtt::texture {
    auto constexpr pdf_dim = 256uz;

    struct Image_Vector_Texture final {
        std::vector<poly<image::Image>> images;

        Image_Vector_Texture(
            poly<image::Image> image
        ) noexcept;

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
        auto fetch(
            math::Vector<i32, 3> texel, Sampler const& sampler
        ) const noexcept -> math::Vector<f32, 4>;
        auto nearest(
            Coordinate const& coord, i32 lod, Sampler const& sampler
        ) const noexcept -> math::Vector<f32, 4>;
        auto linear(
            Coordinate const& coord, i32 lod, Sampler const& sampler
        ) const noexcept -> math::Vector<f32, 4>;

        math::Piecewise_Distribution<pdf_dim / 2, pdf_dim> distr;
    };

    struct Image_Spectrum_Texture final {
        color::Color_Space::Spectrum_Type type;
        Image_Spectrum_Texture(
            poly<image::Image> image,
            color::Color_Space::Spectrum_Type type
        ) noexcept;

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
        Image_Vector_Texture image_tex;
    };
}
