#pragma once
#include <metatron/render/accel/lbvh.hpp>
#include <metatron/render/accel/hwbvh.hpp>

namespace mtt::accel {
    auto init() noexcept -> void;

    struct Acceleration final: stl::polymorph<Acceleration
    , LBVH
    , HWBVH> {
        using polymorph::polymorph;

        auto operator()(math::Ray const& r, fv3 const& n) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return (*p)(r, n); });
        }
    };
}
