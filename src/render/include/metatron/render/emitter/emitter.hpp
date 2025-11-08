#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::emitter {
    struct Divider final {
        view<light::Light> light;
        view<math::Transform> local_to_render;
    };

    struct Interaction final {
        view<Divider> divider;
    };

    MTT_POLY_METHOD(emitter_sample, sample);
    MTT_POLY_METHOD(emitter_sample_infinite, sample_infinite);
    MTT_POLY_METHOD(emitter_pdf, pdf);
    MTT_POLY_METHOD(emitter_pdf_infinite, pdf_infinite);

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
    ::add_convention<emitter_pdf, auto (
        Divider const& divider
    ) const noexcept -> f32>
    ::add_convention<emitter_pdf_infinite, auto (
        Divider const& divider
    ) const noexcept -> f32>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
