#pragma once
#include <metatron/render/material/material.hpp>
#include <metatron/render/media/medium.hpp>
#include <metatron/render/shape/shape.hpp>

namespace mtt::render {

struct [[mtt::reflection]] Divider final {
    Shape shape{};
    Medium int_medium{};
    Medium ext_medium{};
    Material material{};
    core::Transform const* local_to_render{};
    core::Transform const* int_to_render{};
    core::Transform const* ext_to_render{};
};

enum Accel_Flags {
    hit_first = 1 << 0,
    skip_interface = 1 << 1,
    only_interface = 1 << 2,
};

struct Accel_Interaction final {
    Divider const* divider = {};
    u32 primitive = 0;
    fv4 pos = fv4{core::inf<f32>};
};

}
