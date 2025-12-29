#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/distribution/sphere.hpp>
#include <metatron/core/math/distribution/disk.hpp>
#include <metatron/core/math/integral.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/ranges.hpp>

namespace mtt::bsdf {
    auto constexpr fresnel_num_samples = 65536;
    auto constexpr fresnel_length = 256;

    buf<f32> Physical_Bsdf::fresnel_reflectance_table;

    Physical_Bsdf::Physical_Bsdf(
        cref<fv4> reflectance,
        cref<fv4> eta,
        cref<fv4> k,
        f32 alpha_u,
        f32 alpha_v
    ) noexcept:
    reflectance(reflectance),
    eta(eta),
    k(k),
    alpha_u(alpha_u),
    alpha_v(alpha_v) {
        auto has_base = reflectance == math::saturate(reflectance);
        auto has_surface = eta != fv4{0.f};
        auto has_conductor = k != fv4{0.f};

        auto bitmask = has_base | (has_surface << 1) | (has_conductor << 2);
        lambertian = bitmask == 0b001;
        dieletric = bitmask == 0b010;
        conductive = bitmask == 0b110;
        plastic = bitmask == 0b011;
        if (!lambertian && !dieletric && !conductive && ! plastic) {
            stl::print("bsdf not physically possible with these attributes:");
            stl::print("reflectance: {}", reflectance);
            stl::print("eta: {}", eta);
            stl::print("k: {}", k);
            stl::abort();
        }

        fresnel_reflectance = !plastic ? fv4{0.f} :
        math::foreach([](auto eta, auto) {
            auto idx = eta / 3.f * fresnel_length;
            auto low = i32(idx);
            auto high = math::clamp(low + 1, 0, fresnel_num_samples);
            auto alpha = idx - low;
            return math::lerp(fresnel_reflectance_table[low], fresnel_reflectance_table[high], alpha);
        }, eta);
    }

    auto Physical_Bsdf::init() noexcept -> void {
        fresnel_reflectance_table = fresnel_length + 1;
        stl::scheduler::instance().sync_parallel(uzv1{fresnel_length + 1}, [&](auto idx) {
            auto i = idx[0];
            auto integral = 0.0;
            auto eta = i * 3.f / fresnel_length;
            auto f0 = fresnel(0.f, eta, 0.f);
            for (auto i = 1; i <= fresnel_num_samples; ++i) {
                auto cos_theta_2 = f32(i) / f32(fresnel_num_samples);
                auto cos_theta = math::sqrt(cos_theta_2);
                auto f1 = fresnel(cos_theta, eta, 0.f);
                integral += (f0 + f1) * 0.5f / fresnel_num_samples;
            }
            fresnel_reflectance_table[i] = integral;
        });
    }

    auto Physical_Bsdf::operator()(
        cref<fv3> wo, cref<fv3> wi
    ) const noexcept -> opt<Interaction> {
        auto flags = this->flags();
        if (false
        || flags & Flags::specular
        || math::abs(wo[1]) < math::epsilon<f32>
        || math::abs(wi[1]) < math::epsilon<f32>
        ) return {};

        auto reflective = -wo[1] * wi[1] > 0.f;
        auto forward = wi[1] > 0.f;
        auto wm = math::normalize(reflective ? -wo + wi : -wo + wi * eta[0]);
        if (wm[1] < 0.f) wm *= -1.f;
        if (false
        || (!dieletric && (!reflective || !forward))
        || math::abs(wm[1]) < math::epsilon<f32>
        || math::dot(-wo, wm) < 0.f
        || math::dot((reflective ? 1.f : -1.f) * wi, wm) < 0.f) return {};

        if (lambertian) return Interaction{
            .f = lambert(reflectance),
            .wi = wi,
            .pdf = math::Cosine_Hemisphere_Distribution{}.pdf(math::unit_to_cos_theta(wi)),
        };

        auto F = fresnel(math::dot(-wo, wm), eta, k);
        auto D = trowbridge_reitz(wm, alpha_u, alpha_v);
        auto G = smith_shadow(wo, wi, alpha_u, alpha_v);

        auto pr = F[0];
        auto pt = 1.f - F[0];
        pr *= bool(flags & Flags::reflective);
        pt *= bool(flags & Flags::transmissive);
        if (pr == 0.f && pt == 0.f) return {};

        MTT_OPT_OR_RETURN(R, torrance_sparrow(
            reflective, pr, pt,
            F, D, G,
            wo, wi, wm,
            eta, alpha_u, alpha_v
        ), {});

        if (plastic) {
            auto Fi = fresnel(math::unit_to_cos_theta(wi), eta, k);
            auto Fo = fresnel(math::unit_to_cos_theta(-wo), eta, k);
            auto pdf = math::Cosine_Hemisphere_Distribution{}.pdf(math::unit_to_cos_theta(wi));
            auto internal = 1.f
            * (1.f - Fi) * (1.f - Fo) / (math::pi * math::sqr(eta))
            * (reflectance / (1.f - reflectance * fresnel_reflectance));
            R.f += internal;
            R.pdf *= Fo[0];
            R.pdf += (1.f - Fo[0]) * pdf;
        }
        return R;
    }

    auto Physical_Bsdf::sample(
        cref<math::Context> ctx, cref<fv3> u
    ) const noexcept -> opt<Interaction> {
        auto wo = ctx.r.d;
        auto flags = this->flags();
        auto specular = bool(flags & Flags::specular);
        auto Fo = (!specular && !plastic) ? fv4{0.f} :
        fresnel(math::unit_to_cos_theta(-ctx.r.d), eta, k);

        if (specular && (!plastic || u[0] < Fo[0])) {
            auto wm = fv3{0.f, 1.f, 0.f};
            auto cos_theta_o = math::unit_to_cos_theta(-wo);
            if (math::abs(wo[1]) < math::epsilon<f32>) return {};

            auto pr = Fo[0] * bool(flags & Flags::reflective);
            auto pt = (1.f - Fo[0]) * bool(flags & Flags::transmissive);
            if (pr == 0.f && pt == 0.f) return {};

            auto reflective = u[0] < pr / (pr + pt);
            auto wi = reflective ? math::reflect(wo, wm) : math::refract(wo, wm, eta[0]);
            auto cos_theta_i = math::unit_to_cos_theta(wi);
            if (math::abs(wi[1]) < math::epsilon<f32>) return {};

            auto pdf = (reflective ? pr : pt) / (pr + pt);
            auto f = (reflective ? Fo : (1.f - Fo) / math::sqr(eta[0])) / math::abs(cos_theta_i);
            return Interaction{f, wi, pdf};
        } else if (dieletric || conductive || (plastic && u[0] < Fo[0])) {
            if (math::abs(wo[1]) < math::epsilon<f32>) return {};

            auto wy = math::normalize(-wo * fv3{alpha_u, 1.f, alpha_v});
            auto wx = wy[1] < 1.f - math::epsilon<f32>
            ? math::cross(wy, fv3{0.f, 1.f, 0.f})
            : fv3{1.f, 0.f, 0.f};
            auto wz = math::cross(wx, wy);

            // use polar disk distribution to fetch more samples near center
            auto distr = math::Polar_Disk_Distribution{};
            auto sample_p = distr.sample({u[1], u[2]});
            auto sample_h = math::sqrt(1.f - math::sqr(sample_p[0]));
            sample_p[1] = (1.f + wy[1]) / 2.f * sample_p[1] + (1.f - wy[1]) * sample_h / 2.f;

            auto sample_y = math::sqrt(1.f - math::dot(sample_p, sample_p));
            auto wm = sample_p[0] * wx + sample_y * wy + sample_p[1] * wz;
            if (false
            || math::abs(wm[1]) < math::epsilon<f32>
            || math::dot(-wo, wm) < 0.f) return {};
            // normal transformation with inverse transposed matrix
            wm = math::normalize(wm * fv3{alpha_u, 1.f, alpha_v});

            auto F = fresnel(math::dot(-wo, wm), eta, k);
            auto D = trowbridge_reitz(wm, alpha_u, alpha_v);

            auto pr = F[0] * bool(flags & Flags::reflective);
            auto pt = (1.f - F[0]) * bool(flags & Flags::transmissive);
            if (pr == 0.f && pt == 0.f) return {};

            auto reflective = (plastic || conductive) ? true : u[0] < pr / (pr + pt);
            auto wi = reflective ? math::reflect(wo, wm) : math::refract(wo, wm, eta[0]);
            auto G = smith_shadow(wo, wi, alpha_u, alpha_v);
            if (math::abs(wi[1]) < math::epsilon<f32>) return {};

            MTT_OPT_OR_RETURN(R, torrance_sparrow(
                reflective, pr, pt,
                F, D, G,
                wo, wi, wm,
                eta, alpha_u, alpha_v
            ), {});
            R.pdf *= plastic ? Fo[0] : 1.f;
            return R;
        } else {
            auto distr = math::Cosine_Hemisphere_Distribution{};
            auto wi = distr.sample({u[1], u[2]});
            auto pdf = distr.pdf(wi[1]);
            if (math::abs(wi[1]) < math::epsilon<f32>) return {};

            if (lambertian) {
                auto f = lambert(reflectance);
                return Interaction{f, wi, pdf};
            } else {
                auto Fi = fresnel(math::unit_to_cos_theta(wi), eta, k);
                auto f = 1.f
                * (1.f - Fi) * (1.f - Fo) / (math::pi * math::sqr(eta))
                * (reflectance / (1.f - reflectance * fresnel_reflectance));
                pdf *= (1.f - Fo[0]);
                return Interaction{f, wi, pdf};
            }
        }
    }

    auto Physical_Bsdf::flags() const noexcept -> Flags {
        auto flags = 0;
        if (lambertian || plastic || conductive) flags |= Flags::reflective;
        else if (dieletric && eta == fv4{1.f}) flags |= Flags::transmissive;
        else flags |= Flags::transmissive | Flags::reflective;
        if (!lambertian && (alpha_u == 0.f || alpha_v == 0.f)) flags |= Flags::specular;
        return Flags(flags);
    }
}
