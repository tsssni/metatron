#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/math/distribution/discrete.hpp>

namespace mtt::emitter {
    struct Uniform_Emitter final {
        struct Primitive final {
            tag<light::Light> light;
            tag<math::Transform> local_to_render;
        };
        Uniform_Emitter();

        auto sample(
            eval::Context const& ctx,
            f32 u
        ) const noexcept -> std::optional<Interaction>;
        auto sample_infinite(
            eval::Context const& ctx,
            f32 u
        ) const noexcept -> std::optional<Interaction>;

    private:
        std::vector<Primitive> prims;
        std::vector<Primitive> inf_prims;
    };
}
