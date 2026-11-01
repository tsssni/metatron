#pragma once
#include <metatron/core/math/ray.hpp>

namespace mtt::core {

struct Plane final {
    f32 a, b, c, d;
    Plane(f32 a, f32 b, f32 c, f32 d) noexcept;
    Plane(fv3 const& p, fv3 const& n);
};

inline Plane::Plane(f32 a, f32 b, f32 c, f32 d) noexcept
: a(a), b(b), c(c), d(d) {}
inline Plane::Plane(fv3 const& p, fv3 const& n)
: a(n[0]), b(n[1]), c(n[2]), d(-dot(n, p)) {}

auto inline constexpr hit(Ray const& r, Plane const& p) -> f32 {
    auto n = fv3{p.a, p.b, p.c};
    auto no = dot(n, r.o) + p.d;
    auto nd = dot(n, r.d);
    return -no / nd;
}

}
