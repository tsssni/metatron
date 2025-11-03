#include <metatron/resource/texture/checkerboard.hpp>

namespace mtt::texture {
    Checkerboard_Texture::Checkerboard_Texture(Descriptor const& desc) noexcept
    : x(desc.x), y(desc.y), uv_scale(desc.uv_scale) {
        auto CIE_Y = spectra::Spectrum::spectra["CIE-Y"].data();
        auto Y_x = CIE_Y | x.data();
        auto Y_y = CIE_Y | y.data();
        w_x = Y_x / (Y_x + Y_y);
        w_y = Y_y / (Y_x + Y_y);
    }

    auto Checkerboard_Texture::operator()(
        image::Coordinate const& coord,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> spectra::Stochastic_Spectrum {
        auto [u, v] = math::Vector<usize, 2>{coord.uv * uv_scale};
        auto z = ((u + v) % 2 == 0) ? x : y;
        return spec & z.data();
    }

    auto Checkerboard_Texture::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> math::Vector<f32, 2> {
        auto uv = math::Vector<f32, 2>{};
        auto i = 0;
        if (u[0] < w_x) {
            uv = math::Vector<f32, 2>{u[0] / w_x, u[1]};
            i = 0;
        } else {
            uv = math::Vector<f32, 2>{(1.f - u[0]) / w_y, u[1]};
            i = 1;
        }

        uv *= uv_scale;
        auto x = math::mod(1.f, 1.f);
        if (usize(math::sum(math::floor(uv))) % 2 != i)
            uv[0] = math::mod(uv[0] + 1.f, f32(uv_scale[0]));
        return uv;

    }

    auto Checkerboard_Texture::pdf(
        math::Vector<f32, 2> const& uv
    ) const noexcept -> f32 {
        auto [u, v] = math::Vector<usize, 2>{uv * uv_scale};
        return (u + v) % 2 == 0
        ? w_x / math::prod(uv_scale)
        : w_y / math::prod(uv_scale);
    }
}
