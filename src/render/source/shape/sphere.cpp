#include <metatron/core/distribution/sphere.hpp>
#include <metatron/render/shape/sphere.hpp>
#include <metatron/core/distribution/cone.hpp>
#include <metatron/core/math/cmath.hpp>

namespace mtt::render {

auto Sphere::size() const noexcept -> usize {
    return 1uz;
}

auto Sphere::bounding_box(
    core::Transform const& t, usize idx
) const noexcept -> core::Bounding_Box {
    auto bbox = core::Bounding_Box{{-1.f}, {1.f}};
    return t | bbox;
}

auto Sphere::operator()(
    core::Ray const& r, fv3 const& np,
    fv4 const& pos, usize idx
) const noexcept -> Shape_Interaction {
    auto t = pos[3];
    auto p = r.o + t * r.d;
    auto n = p;
    auto [theta, phi] = core::cartesian_to_unit_spherical(p);
    auto uv = fv2{theta / core::pi, phi / (2.f * core::pi)};

    auto dpdu = fv3{
        core::cos(theta) * core::cos(phi),
        -core::sin(theta),
        core::cos(theta) * core::sin(phi),
    };
    auto dpdv = fv3{
        -core::sin(theta) * core::sin(phi),
        0.f,
        core::sin(theta) * core::cos(phi),
    };
    dpdu /= core::pi;
    dpdu /= (2.f * core::pi);

    auto dndu = dpdu;
    auto dndv = dpdv;
    auto tn = core::gram_schmidt(core::normalize(dpdu), n);
    auto bn = core::cross(tn, n);

    auto pdf = core::length(r.o) < 1.f
        ? core::Sphere_Distribution{}.pdf()
        : core::Cone_Distribution{core::pow<1, 2>(1.f - 1.f / core::dot(r.o, r.o))}.pdf();

    return {p, n, tn, bn, uv, t, pdf, dpdu, dpdv, dndu, dndv};
}

auto Sphere::sample(
    core::Ray_Context const& ctx, fv2 const& u, usize idx
) const noexcept -> Shape_Interaction {
    auto d = core::length(ctx.r.o);
    auto r = core::Ray{};
    if (d < 1.f) {
        auto distr = core::Sphere_Distribution{};
        auto p = distr.sample(u);
        auto dir = core::normalize(p - ctx.r.o);
        r = core::Ray{ctx.r.o, dir};
    } else {
        auto cos_theta_max = core::pow<1, 2>(1.f - 1.f / (d * d));
        auto distr = core::Cone_Distribution{cos_theta_max};
        auto dir = core::normalize(-ctx.r.o);
        auto sdir = distr.sample(u);
        auto rot = fq::from_rotation_between({0.f, 0.f, 1.f}, dir);
        sdir = core::rotate(core::expand(sdir, 0.f), rot);
        r = core::Ray{ctx.r.o, sdir};
    }
    return (*this)(r, ctx.n, query(r));
}

auto Sphere::query(
    core::Ray const& r, usize idx
) const noexcept -> fv4 {
    auto a = core::dot(r.d, r.d);
    auto b = core::dot(r.o, r.d) * 2.f;
    auto c = core::dot(r.o, r.o) - 1.f;

    auto delta = b * b - 4.f * a * c;
    if (delta < 0.f) return fv4{core::inf<f32>};

    auto x0 = (-b - core::pow<1, 2>(delta)) / (2.f * a);
    auto x1 = (-b + core::pow<1, 2>(delta)) / (2.f * a);
    if (x1 < 0.f) return fv4{core::inf<f32>};
    return x0 < 0.f ? fv4{0.f, 0.f, -1.f, x1} : fv4{0.f, 0.f, 1.f, x0};
}

}
