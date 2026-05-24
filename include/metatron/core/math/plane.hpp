#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::math {
    struct Plane final {
        f32 a, b, c, d;

        Plane(f32 a, f32 b, f32 c, f32 d) noexcept:
        a(a), b(b), c(c), d(d) {}

        Plane(cref<fv3> p, cref<fv3> n):
        a(n[0]), b(n[1]), c(n[2]), d(-math::dot(n, p)) {}
    };

    auto inline constexpr hit(cref<Ray> r, cref<Plane> p) -> opt<f32> {
        auto n = fv3{p.a, p.b, p.c};
        auto no = math::dot(n, r.o) + p.d;
        auto nd = math::dot(n, r.d);
        if (nd == 0.f && no != 0.f) return {};
        return -no / nd;
    }
}
