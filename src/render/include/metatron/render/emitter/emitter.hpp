#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::emitter {
    struct Divider final {
        view<light::Light> light;
        math::Transform const* local_to_world;
    };

    struct Interaction final {
        Divider const* divider;
        f32 pdf{0.f};
    };

    MTT_POLY_METHOD(emitter_sample, sample);
    MTT_POLY_METHOD(emitter_sample_infinite, sample_infinite);

    struct Emitter final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        eval::Context const& ctx,
        Divider const& divider
    ) const noexcept -> std::optional<Interaction>>
    ::add_convention<emitter_sample, auto (
        eval::Context const& ctx,
        f32 u
    ) const noexcept -> std::optional<Interaction>>
    ::add_convention<emitter_sample_infinite, auto (
        eval::Context const& ctx,
        f32 u
    ) const noexcept -> std::optional<Interaction>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
