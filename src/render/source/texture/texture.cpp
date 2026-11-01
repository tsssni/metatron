#include <metatron/render/texture/texture.hpp>
#include <metatron/core/math/plane.hpp>

namespace mtt::render {

auto grad(
    core::Ray_Differential const& diff,
    render::Shape_Interaction const& intr
) noexcept -> fv4 {
    auto tangent = core::Plane{intr.p, intr.n};
    auto dt = core::hit(diff.r, tangent);
    auto dxt = core::hit(diff.rx, tangent);
    auto dyt = core::hit(diff.ry, tangent);

    auto p = diff.r.o + dt * diff.r.d;
    auto dpdx = diff.rx.o + dxt * diff.rx.d - p;
    auto dpdy = diff.ry.o + dyt * diff.ry.d - p;
    auto dpduv = core::transpose(fm23{intr.dpdu, intr.dpdv});
    auto duvdx = core::least_squares(dpduv, dpdx);
    auto duvdy = core::least_squares(dpduv, dpdy);

    return {duvdx[0], duvdy[0], duvdx[1], duvdy[1]};
}

auto propagate(
    core::Ray_Differential const& diff,
    render::Shape_Interaction const& intr,
    fv4 const& duvdxy,
    fv3 const& wi, fv4 const& eta
) noexcept -> core::Ray_Differential {
    auto wo = -core::normalize(diff.r.d);
    auto reflective = core::dot(wo, intr.n) * core::dot(wi, intr.n) > 0.f;
    auto n = core::normalize(reflective ? wo + wi : wo + eta[0] * wi);
    n = core::dot(n, wo) < 0.f ? -n : n;

    auto p = intr.p;
    auto dpdx = intr.dpdu * duvdxy[0] + intr.dpdv * duvdxy[2];
    auto dpdy = intr.dpdu * duvdxy[1] + intr.dpdv * duvdxy[3];
    auto dndx = intr.dndu * duvdxy[0] + intr.dndv * duvdxy[2];
    auto dndy = intr.dndu * duvdxy[1] + intr.dndv * duvdxy[3];

    auto derive = [&](core::Ray const& r, fv3 const& dpdx, fv3 const& dndx) -> core::Ray {
        auto d = core::normalize(r.d);
        auto m = core::normalize(n + dndx);
        auto wx = reflective ? core::reflect(d, m) : core::refract(d, m, eta[0]);
        return {p + dpdx, core::normalize(wx)};
    };

    return {{p, wi}, derive(diff.rx, dpdx, dndx), derive(diff.ry, dpdy, dndy)};
}

}
