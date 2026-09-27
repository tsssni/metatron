#pragma once
#include <metatron/render/accel/lbvh.hpp>
#include <metatron/render/accel/hwbvh.hpp>

namespace mtt::accel {
    auto init() noexcept -> void;

    struct Acceleration final: stl::polynomial<Acceleration
    , LBVH
    , HWBVH> {
        using polynomial::polynomial;

        auto operator()(cref<math::Ray> r, cref<fv3> n) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return (*p)(r, n); });
        }
    };
}
