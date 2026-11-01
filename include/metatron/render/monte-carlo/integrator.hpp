#pragma once
#include <metatron/render/monte-carlo/radiative.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::render {

struct Integrator final : core::protocol<Radiative_Integrator> {
    using protocol::protocol;

    auto trace(core::recorder& r, Monte_Carlo_Context& ctx) noexcept -> void;
};

auto inline Integrator::trace(core::recorder& r, Monte_Carlo_Context& ctx) noexcept -> void {
    return visit([&](auto* p) noexcept { return p->trace(r, ctx); });
}

}
