#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::math {
    struct Plane final {
        f32 a, b, c, d;

        Plane(f32 a, f32 b, f32 c, f32 d) noexcept:
        a(a), b(b), c(c), d(d) {}

        Plane(math::Vector<f32, 3> const& p, math::Vector<f32, 3> const& n):
        a(n[0]), b(n[1]), c(n[2]), d(-math::dot(n, p)) {}
    };

    auto inline constexpr hit(Ray const& r, Plane const& p) -> std::optional<f32> {
        auto n = math::Vector<f32, 3>{p.a, p.b, p.c};
        auto no = math::dot(n, r.o) + p.d;
        auto nd = math::dot(n, r.d);
        if (nd == 0.f && no != 0.f) {
            return {};
        }
        return -no / nd;
    }
}
