#pragma once
#include <metatron/resource/serde/hierarchy.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/media/medium.hpp>

namespace mtt::accel {
    auto constexpr default_medium = "/medium/vaccum";
    auto constexpr default_transform = "/hierarchy/medium/vaccum";

    struct Divider final {
        shape::Shape shape{};
        media::Medium int_medium{media::Medium::entity(default_medium)};
        media::Medium ext_medium{media::Medium::entity(default_medium)};
        material::Material material{};
        math::proxy::Transform local_to_render{};
        math::proxy::Transform int_to_render{math::proxy::Transform::entity(default_transform)};
        math::proxy::Transform ext_to_render{math::proxy::Transform::entity(default_transform)};
    };
}

namespace mtt::accel::proxy {
    struct Divider: stl::proxy<Divider, accel::Divider> { using proxy::proxy; };
}

namespace mtt::accel {
    struct Interaction final {
        proxy::Divider divider = {};
        u32 primitive = 0;
        opt<shape::Interaction> intr_opt = {};
    };
}
