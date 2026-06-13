#pragma once
#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/texture/image.hpp>
#include <metatron/resource/texture/checkerboard.hpp>
#include <metatron/resource/shape/interaction.hpp>

namespace mtt::texture {
    struct Spectrum_Texture final: stl::polynomial<Spectrum_Texture
    , Constant_Spectrum_Texture
    , Image_Spectrum_Texture
    , Checkerboard_Texture> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto operator()(cref<muldim::Coordinate> coord, cref<fv4> lambda) const noexcept -> fv4 {
            return visit([&](auto* p) noexcept { return (*p)(coord, lambda); });
        }
        auto sample(cref<math::Context> ctx, cref<fv2> u) const noexcept -> fv2 {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto pdf(cref<fv2> uv) const noexcept -> f32 {
            return visit([&](auto* p) noexcept { return p->pdf(uv); });
        }
    };

    struct Vector_Texture final: stl::polynomial<Vector_Texture
    , Constant_Vector_Texture
    , Image_Vector_Texture> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto operator()(cref<muldim::Coordinate> coord) const noexcept -> fv4 {
            return visit([&](auto* p) noexcept { return (*p)(coord); });
        }
        auto sample(cref<math::Context> ctx, fv2 u) const noexcept -> fv2 {
            return visit([&, u](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto pdf(fv2 uv) const noexcept -> f32 {
            return visit([uv](auto* p) noexcept { return p->pdf(uv); });
        }
    };

    auto grad(
        cref<math::Ray_Differential> diff,
        cref<shape::Interaction> intr
    ) noexcept -> opt<muldim::Coordinate>;
}
