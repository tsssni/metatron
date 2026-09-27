#pragma once
#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/texture/checkerboard.hpp>
#include <metatron/resource/shape/interaction.hpp>

namespace mtt::texture {
    auto init() noexcept -> void;

    struct Spectrum_Texture final: stl::polynomial<Spectrum_Texture
    , Constant_Spectrum_Texture
    , Image_Spectrum_Texture
    , Checkerboard_Texture> {
        using polynomial::polynomial;

        auto operator()(muldim::Coordinate const& coord, fv4 const& lambda) const noexcept -> fv4 {
            return visit([&](auto* p) noexcept { return (*p)(coord, lambda); });
        }
        auto sample(math::Context const& ctx, fv2 const& u) const noexcept -> fv2 {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto pdf(fv2 const& uv) const noexcept -> f32 {
            return visit([&](auto* p) noexcept { return p->pdf(uv); });
        }
    };

    struct Vector_Texture final: stl::polynomial<Vector_Texture
    , Constant_Vector_Texture
    , Image_Vector_Texture> {
        using polynomial::polynomial;

        auto operator()(muldim::Coordinate const& coord) const noexcept -> fv4 {
            return visit([&](auto* p) noexcept { return (*p)(coord); });
        }
        auto sample(math::Context const& ctx, fv2 u) const noexcept -> fv2 {
            return visit([&, u](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto pdf(fv2 uv) const noexcept -> f32 {
            return visit([uv](auto* p) noexcept { return p->pdf(uv); });
        }
    };

    auto grad(
        math::Ray_Differential const& diff,
        shape::Interaction const& intr
    ) noexcept -> muldim::Coordinate;

    auto propagate(
        math::Ray_Differential const& diff,
        shape::Interaction const& intr,
        muldim::Coordinate const& coord,
        fv3 const& wi, fv4 const& eta
    ) noexcept -> math::Ray_Differential;
}
