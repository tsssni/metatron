#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/spectra/color-space.hpp>
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
            Image_Distribution distr = Image_Distribution::none;
            bool linear = true;
        };
        Image_Vector_Texture() noexcept = default;
        Image_Vector_Texture(cref<Descriptor> desc) noexcept;

        auto operator()(
            cref<opaque::Coordinate> coord
        ) const noexcept -> fv4;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;

    private:
        tag<opaque::Image> texture;
        math::Piecewise_Distribution<2> distr;
    };

    struct Image_Spectrum_Texture final {
        struct Descriptor final {
            std::string path;
            spectra::Color_Space::Spectrum_Type type;
            Image_Distribution distr = Image_Distribution::none;
            tag<spectra::Color_Space> color_space = spectra::Color_Space::color_spaces["sRGB"];
        };
        Image_Spectrum_Texture() noexcept = default;
        Image_Spectrum_Texture(cref<Descriptor> desc) noexcept;

        auto operator()(
            cref<opaque::Coordinate> coord, cref<fv4> spec
        ) const noexcept -> fv4;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;

    private:
        tag<spectra::Color_Space> color_space;
        spectra::Color_Space::Spectrum_Type type;
        Image_Vector_Texture image_tex;
    };
}
