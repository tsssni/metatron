#pragma once
#include <metatron/render/emitter/uniform.hpp>

namespace mtt::emitter {
    auto init() noexcept -> void;

    struct Emitter final: stl::polymorph<Emitter
    , Uniform_Emitter> {
        using polymorph::polymorph;

        auto sample(math::Context const& ctx, f32 u) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto sample_infinite(math::Context const& ctx, f32 u) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return p->sample_infinite(ctx, u); });
        }
    };
}
