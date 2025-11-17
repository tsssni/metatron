#include <metatron/resource/shape/sphere.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/distribution/sphere.hpp>
#include <metatron/core/math/distribution/cone.hpp>

namespace mtt::shape {
    Sphere::Sphere(cref<Descriptor> desc) noexcept {}

    auto Sphere::size() const noexcept -> usize {
        return 1uz;
    }

    auto Sphere::bounding_box(
        cref<fm44> t, usize idx
    ) const noexcept -> math::Bounding_Box {
        auto c = t | fv4{0.f, 0.f, 0.f, 1.f};
        auto d = t | fv4{0.f, 1.f, 0.f, 0.f};
        auto r = fv3{math::length(d - c)};
        return {c - r, c + r};
    }

    auto Sphere::operator()(
        cref<math::Ray> r, cref<fv3> np, usize idx
    ) const noexcept -> opt<Interaction> {
        MTT_OPT_OR_RETURN(t, query(r, idx), {});
        auto p = r.o + t * r.d;
        auto n = p;

        auto s = math::cartesian_to_unit_spherical(p);
        auto& theta = s[0];
        auto& phi = s[1];
        auto uv = fv2{
            theta / math::pi,
            phi / (2.f * math::pi)
        };

        auto dpdu = fv3{
            std::cos(theta) * std::cos(phi),
            -std::sin(theta),
            std::cos(theta) * std::sin(phi),
        } / math::pi;
        auto dpdv = fv3{
            -std::sin(theta) * std::sin(phi),
            0.f,
            std::sin(theta) * std::cos(phi),
        } / (2.f * math::pi);
        auto dndu = dpdu;
        auto dndv = dpdv;

        auto tn = math::normalize(dpdu);
        tn = math::gram_schmidt(tn, n);
        auto bn = math::cross(tn, n);

        auto pdf = 1.f;
        auto d = math::length(r.o);
        if (d < 1.f) {
            pdf = math::Sphere_Distribution{}.pdf();
        } else {
            auto cos_theta_max = math::sqrt(1.f - 1.f / (d * d));
            pdf = math::Cone_Distribution{cos_theta_max}.pdf();
        }

        return Interaction{p, n, tn, bn, uv, t, pdf, dpdu, dpdv, dndu, dndv};
    }

    auto Sphere::sample(
        cref<math::Context> ctx, cref<fv2> u, usize idx
    ) const noexcept -> opt<Interaction> {
        auto d = math::length(ctx.r.o);
        if (d < 1.f) {
            auto distr = math::Sphere_Distribution{};
            auto p = distr.sample(u);
            auto dir = math::normalize(p - ctx.r.o);

            return (*this)({ctx.r.o, dir}, ctx.n);
        } else {
            auto cos_theta_max = math::sqrt(1.f - 1.f / (d * d));
            auto distr = math::Cone_Distribution{cos_theta_max};
            auto dir = math::normalize(-ctx.r.o);

            auto sdir = distr.sample(u);
            auto rot = fq::from_rotation_between({0.f, 0.f, 1.f}, dir);
            sdir = math::rotate(math::expand(sdir, 0.f), rot);

            return (*this)({ctx.r.o, sdir}, ctx.n);
        }
    }

    auto Sphere::query(
        cref<math::Ray> r, usize idx
    ) const noexcept -> opt<f32> {
        auto a = math::dot(r.d, r.d);
        auto b = math::dot(r.o, r.d) * 2.f;
        auto c = math::dot(r.o, r.o) - 1.f;

        auto delta = b * b - 4.f * a * c;
        if (delta < 0.f) return {};

        auto x0 = (-b - math::sqrt(delta)) / (2.f * a);
        auto x1 = (-b + math::sqrt(delta)) / (2.f * a);
        if (x1 < 0.f) return {};
        return x0 < 0.f ? x1 : x0;
    }
}
