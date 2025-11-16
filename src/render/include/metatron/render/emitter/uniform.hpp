#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/stl/arena.hpp>

namespace mtt::emitter {
    struct Uniform_Emitter final {
        struct Primitive final {
            tag<light::Light> light;
            tag<math::Transform> local_to_render;
        };
        Uniform_Emitter();

        auto sample(
            cref<eval::Context> ctx, f32 u
        ) const noexcept -> opt<Interaction>;
        auto sample_infinite(
            cref<eval::Context> ctx, f32 u
        ) const noexcept -> opt<Interaction>;

    private:
        buf<Primitive> prims;
        buf<Primitive> inf_prims;
    };
}
