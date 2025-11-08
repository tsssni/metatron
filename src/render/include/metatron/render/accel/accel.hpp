#pragma once
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::accel {
    auto constexpr default_medium = "/hierarchy/medium/vaccum";

    struct Divider final {
        proxy<shape::Shape> shape{};
        proxy<media::Medium> int_medium{scene::fetch<media::Medium>(default_medium / et)};
        proxy<media::Medium> ext_medium{scene::fetch<media::Medium>(default_medium / et)};
        proxy<material::Material> material{};
        proxy<math::Transform> local_to_render{};
        proxy<math::Transform> int_to_render{scene::fetch<math::Transform>(default_medium / et)};
        proxy<math::Transform> ext_to_render{scene::fetch<math::Transform>(default_medium / et)};
    };

    struct Interaction final {
        proxy<Divider> divider{};
        usize primitive{0uz};
        std::optional<shape::Interaction> intr_opt{};
    };

    struct Acceleration final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        math::Ray const& r,
        math::Vector<f32, 3> const& n
    ) const noexcept -> std::optional<Interaction>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
