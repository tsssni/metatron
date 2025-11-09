#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/resource/spectra/constant.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/distribution/sphere.hpp>
#include <metatron/core/math/distribution/disk.hpp>
#include <metatron/core/math/integral.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/ranges.hpp>

namespace mtt::bsdf {
    auto constexpr fresnel_num_samples = 65536;
    auto constexpr fresnel_length = 256;

    std::vector<f32> Physical_Bsdf::fresnel_reflectance_table;

    Physical_Bsdf::Physical_Bsdf(
        spectra::Stochastic_Spectrum const& spectrum,
        spectra::Stochastic_Spectrum const& reflectance,
        spectra::Stochastic_Spectrum const& eta,
        spectra::Stochastic_Spectrum const& k,
        f32 alpha_u,
        f32 alpha_v,
        bool inside
    ) noexcept {
        auto has_base = spectra::valid(reflectance);
        auto has_surface = spectra::valid(eta);
        auto has_conductor = spectra::valid(k);
        auto null_spec = spectrum & spectra::Spectrum::spectra["zero"];
        auto invalid_spec = spectra::Stochastic_Spectrum{};

        auto bitmask = has_base | (has_surface << 1) | (has_conductor << 2);
        lambertian = bitmask == 0b001;
        dieletric = bitmask == 0b010;
        conductive = bitmask == 0b110;
        plastic = bitmask == 0b011;
        if (!lambertian && !dieletric && !conductive && ! plastic) {
            std::println("bsdf not physically possible with these attributes:");
            std::abort();
        }

        if (has_surface) {
            this->eta = eta;
            this->eta = inside ? 1.f / eta : eta;
            this->alpha_u = alpha_u;
            this->alpha_v = alpha_v;
        }
        if (plastic) {
            fresnel_reflectance = eta;
            fresnel_reflectance.value = math::foreach([](auto eta, auto) {
                auto idx = eta / 3.f * fresnel_length;
                auto low = i32(idx);
                auto high = math::clamp(low + 1, 0, fresnel_num_samples);
                auto alpha = idx - low;
                return math::lerp(fresnel_reflectance_table[low], fresnel_reflectance_table[high], alpha);
            }, eta.value);
        }
        this->reflectance = has_base ? reflectance : invalid_spec;
        this->k = has_conductor ? k : null_spec;
    }

    auto Physical_Bsdf::init() noexcept -> void {
        fresnel_reflectance_table.resize(fresnel_length + 1);
        stl::scheduler::instance().sync_parallel(math::Vector<usize, 1>{fresnel_length + 1}, [&](auto idx) {
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
        auto& m = *(math::Vector<f32, fresnel_length + 1>*)(fresnel_reflectance_table.data());
    }

    auto Physical_Bsdf::operator()(
        math::Vector<f32, 3> const& wo,
        math::Vector<f32, 3> const& wi
    ) const noexcept -> std::optional<Interaction> {
        if (false
        || math::abs(wo[1]) < math::epsilon<f32>
        || math::abs(wi[1]) < math::epsilon<f32>
        ) {
            return {};
        }

        auto reflective = -wo[1] * wi[1] > 0.f;
        auto forward = wi[1] > 0.f;
        auto wm = math::normalize(reflective ? -wo + wi : -wo + wi * eta.value[0]);
        if (wm[1] < 0.f) wm *= -1.f;
        if (false
        || (!dieletric && (!reflective || !forward))
        || math::abs(wm[1]) < math::epsilon<f32>
        || math::dot(-wo, wm) < 0.f
        || math::dot((reflective ? 1.f : -1.f) * wi, wm) < 0.f) return {};

        if (lambertian)
            return Interaction{
                .f = lambert(reflectance),
                .wi = wi,
                .pdf = math::Cosine_Hemisphere_Distribution{}.pdf(math::unit_to_cos_theta(wi)),
            };

        auto F = fresnel(math::dot(-wo, wm), eta, k);
        auto D = trowbridge_reitz(wm, alpha_u, alpha_v);
        auto G = smith_shadow(wo, wi, alpha_u, alpha_v);

        auto pr = F.value[0];
        auto pt = 1.f - F.value[0];
        auto flags = this->flags();
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
            R.pdf *= Fo.value[0];
            R.pdf += (1.f - Fo.value[0]) * pdf;
        }
        return R;
    }

    auto Physical_Bsdf::sample(
        eval::Context const& ctx,
        math::Vector<f32, 3> const& u
    ) const noexcept -> std::optional<Interaction> {
        auto Fo = plastic ? fresnel(math::unit_to_cos_theta(-ctx.r.d), eta, k) : 0.f;
        if (dieletric || conductive || (plastic && u[0] < Fo.value[0])) {
            auto wo = ctx.r.d;
            if (math::abs(wo[1]) < math::epsilon<f32>) return {};

            auto wy = math::normalize(-wo * math::Vector<f32, 3>{alpha_u, 1.f, alpha_v});
            auto wx = wy[1] < 1.f - math::epsilon<f32>
            ? math::cross(wy, math::Vector<f32, 3>{0.f, 1.f, 0.f})
            : math::Vector<f32, 3>{1.f, 0.f, 0.f};
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
            wm = math::normalize(wm * math::Vector<f32, 3>{alpha_u, 1.f, alpha_v});

            auto F = fresnel(math::dot(-wo, wm), eta, k);
            auto D = trowbridge_reitz(wm, alpha_u, alpha_v);

            auto pr = F.value[0];
            auto pt = 1.f - F.value[0];
            auto flags = this->flags();
            pr *= bool(flags & Flags::reflective);
            pt *= bool(flags & Flags::transmissive);
            if (pr == 0.f && pt == 0.f) return {};

            auto reflective = (plastic || conductive) ? true : u[0] < pr / (pr + pt);
            auto wi = reflective ? math::reflect(wo, wm) : math::refract(wo, wm, eta.value[0]);
            auto G = smith_shadow(wo, wi, alpha_u, alpha_v);
            if (math::abs(wi[1]) < math::epsilon<f32>) return {};

            MTT_OPT_OR_RETURN(R, torrance_sparrow(
                reflective, pr, pt,
                F, D, G,
                wo, wi, wm,
                eta, alpha_u, alpha_v
            ), {});
            R.pdf *= plastic ? Fo.value[0] : 1.f;
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
                pdf *= (1.f - Fo.value[0]);
                return Interaction{f, wi, pdf};
            }
        }
    }

    auto Physical_Bsdf::degrade() noexcept -> bool {
        if (spectra::max(k) == 0.f && !spectra::constant(eta)) {
            spectra::degrade(eta);
            spectra::degrade(k);
            return true;
        } else {
            return false;
        }
    }

    auto Physical_Bsdf::flags() const noexcept -> Flags {
        auto flags = 0;
        if (spectra::valid(reflectance) || spectra::max(k) > 0.f)
            flags |= Flags::reflective;
        else if (spectra::constant(eta) && eta.value[0] == 1.f)
            flags |= Flags::transmissive;
        else
            flags |= (Flags::transmissive | Flags::reflective);
        return Flags(flags);
    }
}
