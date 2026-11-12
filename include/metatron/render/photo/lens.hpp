#pragma once
#include <metatron/render/photo/pinhole.hpp>
#include <metatron/render/photo/thin.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::render {

struct Lens final : core::protocol<Pinhole_Lens, Thin_Lens> {
    using protocol::protocol;

    auto sample(fv2 const& o, fv2 const& u) const noexcept -> Lens_Interaction;
};

auto inline Lens::sample(fv2 const& o, fv2 const& u) const noexcept -> Lens_Interaction {
    return visit([&](auto* p) noexcept { return p->sample(o, u); });
}

}
