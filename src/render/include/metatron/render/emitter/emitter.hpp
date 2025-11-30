#pragma once
#include <metatron/render/accel/accel.hpp>
#include <metatron/resource/light/light.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::emitter {
    struct Interaction final {
        tag<light::Light> light;
        tag<math::Transform> local_to_render;
        f32 pdf;
    };

    MTT_POLY_METHOD(emitter_sample, sample);
    MTT_POLY_METHOD(emitter_sample_infinite, sample_infinite);

    struct Emitter final: pro::facade_builder
    ::add_convention<emitter_sample, auto (
        cref<math::Context> ctx, f32 u
    ) const noexcept -> opt<Interaction>>
    ::add_convention<emitter_sample_infinite, auto (
        cref<math::Context> ctx, f32 u
    ) const noexcept -> opt<Interaction>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
