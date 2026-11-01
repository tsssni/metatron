#pragma once
#include <metatron/render/emitter/uniform.hpp>

namespace mtt::render {

struct Emitter final : core::protocol<Uniform_Emitter> {
    using protocol::protocol;

    auto sample(core::Ray_Context const& ctx, f32 u) const noexcept -> Emitter_Interaction;
    auto sample_infinite(core::Ray_Context const& ctx, f32 u) const noexcept -> Emitter_Interaction;
};

auto inline Emitter::sample(core::Ray_Context const& ctx, f32 u) const noexcept -> Emitter_Interaction {
    return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
}

auto inline Emitter::sample_infinite(core::Ray_Context const& ctx, f32 u) const noexcept -> Emitter_Interaction {
    return visit([&](auto* p) noexcept { return p->sample_infinite(ctx, u); });
}

}
