#pragma once
#include <metatron/resource/shape/sphere.hpp>
#include <metatron/resource/eval/context.hpp>

namespace mtt::texture {
    struct Coordinate final {
        math::Vector<f32, 2> uv{};
        f32 dudx{0.f};
        f32 dudy{0.f};
        f32 dvdx{0.f};
        f32 dvdy{0.f};
    };

    MTT_POLY_METHOD(texture_sample, sample);

    struct Spectrum_Texture final: pro::facade_builder
    ::add_convention<texture_sample, auto (
        eval::Context const& ctx,
        Coordinate const& coord
    ) const noexcept -> spectra::Stochastic_Spectrum>
    ::template add_skill<pro::skills::as_view>
    ::build {};

    struct Vector_Texture final: pro::facade_builder
    ::add_convention<texture_sample, auto (
        eval::Context const& ctx,
        Coordinate const& coord
    ) const noexcept -> math::Vector<f32, 4>>
    ::template add_skill<pro::skills::as_view>
    ::build {};

    auto grad(
        math::Ray_Differential const& diff,
        shape::Interaction const& intr
    ) noexcept -> std::optional<texture::Coordinate>;
}
