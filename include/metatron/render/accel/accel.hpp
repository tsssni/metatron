#pragma once
#include <metatron/render/accel/lbvh.hpp>

namespace mtt::render {

struct Acceleration final : core::protocol<LBVH> {
    using protocol::protocol;

    auto operator()(
        core::Ray const& r, fv3 const& n,
        Accel_Flags flags, fv2 const& range
    ) const noexcept -> Accel_Interaction;
};

auto inline Acceleration::operator()(
    core::Ray const& r, fv3 const& n,
    Accel_Flags flags, fv2 const& range
) const noexcept -> Accel_Interaction {
    return visit([&](auto* p) noexcept { return (*p)(r, n, flags, range); });
}

}
