#pragma once
#include <metatron/render/spectra/color-space.hpp>
#include <metatron/core/distribution/piecewise.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>
#include <string_view>

namespace mtt::render {

enum struct Image_Distribution {
    none,
    uniform,
    spherical,
};

struct Image_Vector_Texture final {
    ft2 texture;
    core::Piecewise_Distribution distr;
    Image_Vector_Texture(std::string_view path, Image_Distribution distribution, bool linear, core::recorder& recorder) noexcept;
    Image_Vector_Texture(ft2 texture, Image_Distribution distribution, core::recorder& recorder) noexcept;

    auto operator()(
        fv2 const& uv, fv4 const& duvdxy
    ) const noexcept -> fv4;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> fv2;
    auto pdf(fv2 const& uv) const noexcept -> f32;
};

struct Image_Spectrum_Texture final {
    render::Color_Space const* color_space;
    render::Color_Space::Spectrum_Type type;
    Image_Vector_Texture image_tex;

    Image_Spectrum_Texture(
        std::string_view path,
        render::Color_Space::Spectrum_Type type,
        Image_Distribution distribution,
        render::Color_Space const* color_space,
        core::recorder& recorder
    ) noexcept;

    auto operator()(
        fv2 const& uv, fv4 const& duvdxy, fv4 const& spec
    ) const noexcept -> fv4;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> fv2;
    auto pdf(fv2 const& uv) const noexcept -> f32;
};

}
