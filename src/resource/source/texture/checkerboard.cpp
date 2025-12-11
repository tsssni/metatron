#include <metatron/resource/texture/checkerboard.hpp>

namespace mtt::texture {
    Checkerboard_Texture::Checkerboard_Texture(cref<Descriptor> desc) noexcept:
    x(desc.x), y(desc.y), uv_scale(desc.uv_scale) {
        auto CIE_Y = entity<spectra::Spectrum>("/spectrum/CIE-Y");
        auto Y_x = CIE_Y | x;
        auto Y_y = CIE_Y | y;
        w_x = Y_x / (Y_x + Y_y);
        w_y = Y_y / (Y_x + Y_y);
    }

    auto Checkerboard_Texture::operator()(
        cref<muldim::Coordinate> coord, cref<fv4> spec
    ) const noexcept -> fv4 {
        auto [u, v] = uzv2{coord.uv * uv_scale};
        auto z = ((u + v) % 2 == 0) ? x : y;
        return spec & z;
    }

    auto Checkerboard_Texture::sample(
        cref<math::Context> ctx, cref<fv2> u
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
        auto x = math::mod(1.f, 1.f);
        if (usize(math::sum(math::floor(uv))) % 2 != i)
            uv[0] = math::mod(uv[0] + 1.f, f32(uv_scale[0]));
        return uv;
    }

    auto Checkerboard_Texture::pdf(cref<fv2> uv) const noexcept -> f32 {
        auto [u, v] = uzv2{uv * uv_scale};
        return (u + v) % 2 == 0
        ? w_x / math::prod(uv_scale)
        : w_y / math::prod(uv_scale);
    }
}
