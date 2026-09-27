#pragma once
#include <metatron/render/emitter/interaction.hpp>

namespace mtt::emitter {
    struct Uniform_Emitter final {
        struct Primitive final {
            light::Light light;
            math::proxy::Transform local_to_render;
        };

        struct Descriptor final {};
        Uniform_Emitter(Descriptor const&) noexcept;
        Uniform_Emitter() noexcept = default;

        auto sample(
            math::Context const& ctx, f32 u
        ) const noexcept -> Interaction;
        auto sample_infinite(
            math::Context const& ctx, f32 u
        ) const noexcept -> Interaction;

    private:
        buf<Primitive> prims;
        buf<Primitive> inf_prims;
    };
}
