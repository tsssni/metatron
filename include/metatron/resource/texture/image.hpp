#pragma once
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
        Image_Vector_Texture(Descriptor const& desc) noexcept;
        Image_Vector_Texture() noexcept = default;

        auto operator()(
            muldim::Coordinate const& coord
        ) const noexcept -> fv4;
        auto sample(
            math::Context const& ctx, fv2 const& u
        ) const noexcept -> fv2;
        auto pdf(fv2 const& uv) const noexcept -> f32;

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
        Image_Spectrum_Texture(Descriptor const& desc) noexcept;
        Image_Spectrum_Texture() noexcept = default;

        auto operator()(
            muldim::Coordinate const& coord, fv4 const& spec
        ) const noexcept -> fv4;
        auto sample(
            math::Context const& ctx, fv2 const& u
        ) const noexcept -> fv2;
        auto pdf(fv2 const& uv) const noexcept -> f32;

    private:
        color::proxy::Color_Space color_space;
        color::Color_Space::Spectrum_Type type;
        Image_Vector_Texture image_tex;
    };
}
