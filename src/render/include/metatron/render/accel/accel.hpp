#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::accel {
    struct Divider final {
        view<shape::Shape> shape{};
        view<media::Medium> int_medium;
        view<media::Medium> ext_medium;
        view<light::Light> light{};
        view<material::Material> material{};
        view<math::Transform> local_to_render{};
        view<math::Transform> int_to_render{};
        view<math::Transform> ext_to_render{};
        usize primitive{0uz};
    };

    struct Interaction final {
        view<Divider> divider{nullptr};
        std::optional<shape::Interaction> intr_opt;
    };

    struct Acceleration final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        math::Ray const& r
    ) const noexcept -> std::optional<Interaction>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
