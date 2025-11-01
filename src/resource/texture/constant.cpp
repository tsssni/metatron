#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>

namespace mtt::texture {
    auto Constant_Spectrum_Texture::operator()(
        device::Sampler const& sampler,
        Coordinate const& coord,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> spectra::Stochastic_Spectrum {
        return spec & x.data();
    }

    auto Constant_Spectrum_Texture::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> math::Vector<f32, 2> {
        return u;
    }

    auto Constant_Spectrum_Texture::pdf(
        math::Vector<f32, 2> const& uv
    ) const noexcept -> f32 {
        return 1.f;
    }

    auto Constant_Vector_Texture::operator()(
        device::Sampler const& sampler,
        Coordinate const& coord
    ) const noexcept -> math::Vector<f32, 4> {
        return x;
    }

    auto Constant_Vector_Texture::sample(
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> math::Vector<f32, 2> {
        return u;
    }

    auto Constant_Vector_Texture::pdf(
        math::Vector<f32, 2> const& uv
    ) const noexcept -> f32 {
        return 1.f;
    }
}
