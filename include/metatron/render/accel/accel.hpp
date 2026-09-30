#pragma once
#include <metatron/render/accel/lbvh.hpp>

namespace mtt::accel {
    auto init() noexcept -> void;

    struct Acceleration final: stl::polymorph<Acceleration
    , LBVH> {
        using polymorph::polymorph;

        auto operator()(
            math::Ray const& r, fv3 const& n,
            Flags flags, fv2 const& range
        ) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return (*p)(r, n, flags, range); });
        }
    };
}
