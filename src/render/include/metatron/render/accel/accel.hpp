#pragma once
#include <metatron/render/accel/interaction.hpp>
#include <metatron/render/accel/lbvh.hpp>
#include <metatron/render/accel/hwbvh.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::accel {
    struct Acceleration final: stl::polynomial<Acceleration, LBVH, HWBVH> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto operator()(cref<math::Ray> r, cref<fv3> n) const noexcept -> opt<Interaction> {
            return visit([&](auto* p) noexcept { return (*p)(r, n); });
        }
    };
}
