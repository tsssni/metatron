#pragma once
#include <metatron/render/light/area.hpp>
#include <metatron/render/light/atmosphere.hpp>
#include <metatron/render/light/environment.hpp>

namespace mtt::render {

auto init() noexcept -> void;

struct Light final : core::protocol<Area_Light, Environment_Light, Atmosphere_Light> {
    using protocol::protocol;

    auto operator()(core::Ray const& r, fv4 const& lambda) const noexcept -> Light_Interaction;
    auto sample(core::Ray_Context const& ctx, fv2 const& u) const noexcept -> Light_Interaction;
    auto flags() const noexcept -> Light_Flags;
};

auto inline Light::operator()(core::Ray const& r, fv4 const& lambda) const noexcept -> Light_Interaction {
    return visit([&](auto* p) noexcept { return (*p)(r, lambda); });
}

auto inline Light::sample(core::Ray_Context const& ctx, fv2 const& u) const noexcept -> Light_Interaction {
    return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
}

auto inline Light::flags() const noexcept -> Light_Flags {
    return visit([&](auto* p) noexcept { return p->flags(); });
}

template<typename T>
concept Luminous = Light::ts::template contains<T>;

}
