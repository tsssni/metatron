#include <metatron/render/texture/checkerboard.hpp>

namespace mtt::render {

Checkerboard_Texture::Checkerboard_Texture(render::Spectrum x, render::Spectrum y, render::Spectrum CIE_Y, uv2 uv_scale) noexcept
: x(x), y(y), uv_scale(uv_scale) {
    auto Y_x = CIE_Y | x;
    auto Y_y = CIE_Y | y;
    w_x = Y_x / (Y_x + Y_y);
    w_y = Y_y / (Y_x + Y_y);
}

auto Checkerboard_Texture::operator()(
    fv2 const& uv, fv4 const& duvdxy, fv4 const& spec
) const noexcept -> fv4 {
    auto [u, v] = uzv2{uv * uv_scale};
    auto z = ((u + v) % 2 == 0) ? x : y;
    return spec & z;
}

auto Checkerboard_Texture::sample(
    core::Ray_Context const& ctx, fv2 const& u
) const noexcept -> fv2 {
    auto uv = fv2{};
    auto i = 0;
    if (u[0] < w_x) {
        uv = fv2{u[0] / w_x, u[1]};
        i = 0;
    } else {
        uv = fv2{(1.f - u[0]) / w_y, u[1]};
        i = 1;
    }

    uv *= uv_scale;
    if (usize(core::sum(core::floor(uv))) % 2 != i)
        uv[0] = core::mod(uv[0] + 1.f, f32(uv_scale[0]));
    return uv;
}

auto Checkerboard_Texture::pdf(fv2 const& uv) const noexcept -> f32 {
    auto [u, v] = uzv2{uv * uv_scale};
    auto scale = core::prod(uv_scale);
    return (u + v) % 2 == 0 ? w_x / scale : w_y / scale;
}

}
