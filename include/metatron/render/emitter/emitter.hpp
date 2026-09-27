#pragma once
#include <metatron/render/emitter/uniform.hpp>

namespace mtt::emitter {
    auto init() noexcept -> void;

    struct Emitter final: stl::polynomial<Emitter
    , Uniform_Emitter> {
        using polynomial::polynomial;

        auto sample(cref<math::Context> ctx, f32 u) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto sample_infinite(cref<math::Context> ctx, f32 u) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return p->sample_infinite(ctx, u); });
        }
    };
}
