#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>

namespace mtt::texture {
    enum struct Image_Distribution {
        none,
        uniform,
        spherical,
    };

    struct Image_Vector_Texture final {
        struct Descriptor final {
            std::string path;
            Image_Distribution distr;
            bool linear = true;
        };
        Image_Vector_Texture(Descriptor const& desc) noexcept;

        auto operator()(
            device::Coordinate const& coord
        ) const noexcept -> math::Vector<f32, 4>;
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 2> const& u
        ) const noexcept -> math::Vector<f32, 2>;
        auto pdf(
            math::Vector<f32, 2> const& uv
        ) const noexcept -> f32;

    private:
        stl::proxy<device::Texture> texture;
        math::Piecewise_Distribution<2> distr;
    };

    struct Image_Spectrum_Texture final {
        struct Descriptor final {
            std::string path;
            color::Color_Space::Spectrum_Type type;
            Image_Distribution distr;
            stl::proxy<color::Color_Space> color_space = color::Color_Space::color_spaces["sRGB"];
        };
        Image_Spectrum_Texture(Descriptor const& desc) noexcept;

        auto operator()(
            device::Coordinate const& coord,
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
        stl::proxy<color::Color_Space> color_space;
        color::Color_Space::Spectrum_Type type;
        Image_Vector_Texture image_tex;
    };
}
