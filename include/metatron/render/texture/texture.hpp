#pragma once
#include <metatron/render/shape/interaction.hpp>
#include <metatron/render/texture/checkerboard.hpp>
#include <metatron/render/texture/constant.hpp>
#include <metatron/render/texture/image.hpp>

namespace mtt::render {

struct Spectrum_Texture final : core::protocol<Constant_Spectrum_Texture, Image_Spectrum_Texture, Checkerboard_Texture> {
    using protocol::protocol;

    auto operator()(fv2 const& uv, fv4 const& duvdxy, fv4 const& lambda) const noexcept -> fv4;
    auto sample(core::Ray_Context const& ctx, fv2 const& u) const noexcept -> fv2;
    auto pdf(fv2 const& uv) const noexcept -> f32;
};

auto inline Spectrum_Texture::operator()(fv2 const& uv, fv4 const& duvdxy, fv4 const& lambda) const noexcept -> fv4 {
    return visit([&](auto* p) noexcept { return (*p)(uv, duvdxy, lambda); });
}

auto inline Spectrum_Texture::sample(core::Ray_Context const& ctx, fv2 const& u) const noexcept -> fv2 {
    return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
}

auto inline Spectrum_Texture::pdf(fv2 const& uv) const noexcept -> f32 {
    return visit([&](auto* p) noexcept { return p->pdf(uv); });
}

struct Vector_Texture final : core::protocol<Constant_Vector_Texture, Image_Vector_Texture> {
    using protocol::protocol;

    auto operator()(fv2 const& uv, fv4 const& duvdxy) const noexcept -> fv4;
    auto sample(core::Ray_Context const& ctx, fv2 u) const noexcept -> fv2;
    auto pdf(fv2 uv) const noexcept -> f32;
};

auto inline Vector_Texture::operator()(fv2 const& uv, fv4 const& duvdxy) const noexcept -> fv4 {
    return visit([&](auto* p) noexcept { return (*p)(uv, duvdxy); });
}

auto inline Vector_Texture::sample(core::Ray_Context const& ctx, fv2 u) const noexcept -> fv2 {
    return visit([&, u](auto* p) noexcept { return p->sample(ctx, u); });
}

auto inline Vector_Texture::pdf(fv2 uv) const noexcept -> f32 {
    return visit([uv](auto* p) noexcept { return p->pdf(uv); });
}

auto grad(
    core::Ray_Differential const& diff,
    render::Shape_Interaction const& intr
) noexcept -> fv4;

auto propagate(
    core::Ray_Differential const& diff,
    render::Shape_Interaction const& intr,
    fv4 const& duvdxy,
    fv3 const& wi, fv4 const& eta
) noexcept -> core::Ray_Differential;

}
