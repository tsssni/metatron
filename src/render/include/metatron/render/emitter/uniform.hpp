#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::emitter {
    struct Uniform_Emitter final {
        struct Primitive final {
            tag<light::Light> light;
            tag<math::Transform> local_to_render;
        };

        struct Descriptor final {};
        Uniform_Emitter(cref<Descriptor>) noexcept;
        Uniform_Emitter() noexcept = default;

        auto sample(
            cref<math::Context> ctx, f32 u
        ) const noexcept -> opt<Interaction>;
        auto sample_infinite(
            cref<math::Context> ctx, f32 u
        ) const noexcept -> opt<Interaction>;

    private:
        buf<Primitive> prims;
        buf<Primitive> inf_prims;
    };
}
