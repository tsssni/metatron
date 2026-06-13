#pragma once
#include <metatron/render/emitter/uniform.hpp>

namespace mtt::emitter {
    struct Emitter final: stl::polynomial<Emitter
    , Uniform_Emitter> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto sample(cref<math::Context> ctx, f32 u) const noexcept -> opt<Interaction> {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto sample_infinite(cref<math::Context> ctx, f32 u) const noexcept -> opt<Interaction> {
            return visit([&](auto* p) noexcept { return p->sample_infinite(ctx, u); });
        }
    };
}
