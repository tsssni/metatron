#include <metatron/resource/texture/constant.hpp>

namespace mtt::texture {
    auto Constant_Spectrum_Texture::operator()(
        muldim::Coordinate const& coord, fv4 const& spec
    ) const noexcept -> fv4 {
        return spec & x;
    }

    auto Constant_Spectrum_Texture::sample(
        math::Context const& ctx, fv2 const& u
    ) const noexcept -> fv2 {
        return u;
    }

    auto Constant_Spectrum_Texture::pdf(
        fv2 const& uv
    ) const noexcept -> f32 {
        return 1.f;
    }

    auto Constant_Vector_Texture::operator()(
        muldim::Coordinate const& coord
    ) const noexcept -> fv4 {
        return x;
    }

    auto Constant_Vector_Texture::sample(
        math::Context const& ctx, fv2 const& u
    ) const noexcept -> fv2 {
        return u;
    }

    auto Constant_Vector_Texture::pdf(
        fv2 const& uv
    ) const noexcept -> f32 {
        return 1.f;
    }
}
