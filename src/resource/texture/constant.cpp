#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>

namespace mtt::texture {
    Constant_Spectrum_Texture::Constant_Spectrum_Texture(
        view<spectra::Spectrum> x
    ) noexcept: x(x) {}

    auto Constant_Spectrum_Texture::sample(
        eval::Context const& ctx,
        Coordinate const& coord
    ) const noexcept -> spectra::Stochastic_Spectrum {
        return ctx.spec & x;
    }

    Constant_Vector_Texture::Constant_Vector_Texture(
        math::Vector<f32, 4> const& x
    ) noexcept: x(x) {}

    auto Constant_Vector_Texture::sample(
        eval::Context const& ctx,
        Coordinate const& coord
    ) const noexcept -> math::Vector<f32, 4> {
        return x;
    }
}
