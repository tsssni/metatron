#include <metatron/render/texture/constant.hpp>

namespace mtt::render {

auto Constant_Spectrum_Texture::operator()(
    fv2 const& uv, fv4 const& duvdxy, fv4 const& spec
) const noexcept -> fv4 {
    return spec & x;
}

auto Constant_Spectrum_Texture::sample(
    core::Ray_Context const& ctx, fv2 const& u
) const noexcept -> fv2 {
    return u;
}

auto Constant_Spectrum_Texture::pdf(
    fv2 const& uv
) const noexcept -> f32 {
    return 1.f;
}

auto Constant_Vector_Texture::operator()(
    fv2 const& uv, fv4 const& duvdxy
) const noexcept -> fv4 {
    return x;
}

auto Constant_Vector_Texture::sample(
    core::Ray_Context const& ctx, fv2 const& u
) const noexcept -> fv2 {
    return u;
}

auto Constant_Vector_Texture::pdf(
    fv2 const& uv
) const noexcept -> f32 {
    return 1.f;
}

}
