#include <metatron/resource/texture/checkerboard.hpp>

namespace mtt::texture {
    Checkerboard_Texture::Checkerboard_Texture(
        view<spectra::Spectrum> x,
        view<spectra::Spectrum> y,
        math::Vector<usize, 2> uv_scale
    ) noexcept: x(x), y(y), uv_scale(uv_scale) {}

    auto Checkerboard_Texture::sample(
        eval::Context const& ctx,
        Sampler const& sampler,
        Coordinate const& coord
    ) const noexcept -> spectra::Stochastic_Spectrum {
        auto [u, v] = math::Vector<usize, 2>{coord.uv * uv_scale};
        auto z = ((u + v) % 2 == 0) ? x : y;
        return ctx.spec & z;
    }
}
