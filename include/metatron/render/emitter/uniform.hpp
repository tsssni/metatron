#pragma once
#include <metatron/render/emitter/interaction.hpp>

namespace mtt::render {

struct Uniform_Emitter final {
    struct Primitive final {
        render::Light light;
        core::Transform const* local_to_render;
    };

    usize num_prims = 0;
    usize num_inf_prims = 0;
    buf<Primitive> prims;
    buf<Primitive> inf_prims;
    Uniform_Emitter(std::span<Primitive const> primitives) noexcept;

    auto sample(
        core::Ray_Context const& ctx, f32 u
    ) const noexcept -> Emitter_Interaction;
    auto sample_infinite(
        core::Ray_Context const& ctx, f32 u
    ) const noexcept -> Emitter_Interaction;
};

}
