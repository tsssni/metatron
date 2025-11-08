#pragma once
#include <metatron/render/accel/accel.hpp>
#include <metatron/resource/light/light.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::emitter {
    struct Interaction final {
        proxy<light::Light> light;
        proxy<math::Transform> local_to_render;
        f32 pdf;
    };

    MTT_POLY_METHOD(emitter_sample, sample);
    MTT_POLY_METHOD(emitter_sample_infinite, sample_infinite);

    struct Emitter final: pro::facade_builder
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
