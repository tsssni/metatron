#pragma once
#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/muldim/image.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>
#include <metatron/core/math/eval.hpp>

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
        Image_Vector_Texture(cref<Descriptor> desc) noexcept;
        Image_Vector_Texture() noexcept = default;

        auto operator()(
            cref<muldim::Coordinate> coord
        ) const noexcept -> fv4;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;

    private:
        muldim::proxy::Image texture;
        math::proxy::Planar_Distribution distr;
    };

    struct Image_Spectrum_Texture final {
        struct Descriptor final {
            std::string path;
            color::Color_Space::Spectrum_Type type;
            Image_Distribution distr = Image_Distribution::none;
            color::proxy::Color_Space color_space = color::proxy::Color_Space::entity("/color-space/sRGB");
        };
        Image_Spectrum_Texture(cref<Descriptor> desc) noexcept;
        Image_Spectrum_Texture() noexcept = default;

        auto operator()(
            cref<muldim::Coordinate> coord, cref<fv4> spec
        ) const noexcept -> fv4;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;

    private:
        color::proxy::Color_Space color_space;
        color::Color_Space::Spectrum_Type type;
        Image_Vector_Texture image_tex;
    };
}
