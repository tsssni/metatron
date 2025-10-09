#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/complex.hpp>
#include <metatron/core/math/sphere.hpp>

namespace mtt::bsdf {
    auto lambert(f32 reflectance) -> f32 {
        return reflectance / math::pi;
    }

    auto lambert(
        spectra::Stochastic_Spectrum const& reflectance
    ) -> spectra::Stochastic_Spectrum {
        return reflectance / math::pi;
    }

    auto fresnel(f32 cos_theta_i, f32 eta, f32 k) -> f32 {
        cos_theta_i = math::clamp(cos_theta_i, -1.f, 1.f);
        auto F = [cos_theta_i](auto eta) {
            using T = decltype(eta);
            auto constexpr is_complex = std::is_same_v<T, math::Complex<f32>>;

            auto sin2_theta_i = std::max(0.f, 1.f - cos_theta_i * cos_theta_i);
            auto sin2_theta_t = math::guarded_div(sin2_theta_i, eta * eta);
            auto cos_theta_t = math::sqrt(1.f - sin2_theta_t);

            if constexpr (is_complex) {
                auto conductive = eta.i > math::epsilon<f32>;
                if (!conductive && sin2_theta_t.r >= 1.f) {
                    return 1.f;
                }
            }

            auto r_parl = math::guarded_div(
                eta * cos_theta_i - cos_theta_t,
                eta * cos_theta_i + cos_theta_t
            );
            auto r_perp = math::guarded_div(
                cos_theta_i - eta * cos_theta_t,
                cos_theta_i + eta * cos_theta_t
            );
            if constexpr (is_complex) {
                return (math::norm(r_parl) + math::norm(r_perp)) / 2.f;
            } else {
                return (math::sqr(r_parl) + math::sqr(r_perp)) / 2.f;
            }
        };
        
        if (k > math::epsilon<f32>) {
            auto eta_k = math::Complex<f32>{eta, k};
            return F(eta_k);
        } else {
            return F(eta);
        }
    }

    auto fresnel(
        f32 cos_theta_i,
        spectra::Stochastic_Spectrum const& eta,
        spectra::Stochastic_Spectrum const& k
    ) noexcept -> spectra::Stochastic_Spectrum {
        auto F = eta;
        F.value = math::foreach([&](f32 lambda, usize i) {
            return fresnel(cos_theta_i, eta.value[i], k.value[i]);
        }, F.lambda);
        return F;
    }

    auto lambda(
        math::Vector<f32, 3> const& wo,
        f32 alpha_u,
        f32 alpha_v
    ) noexcept -> f32 {
        auto tan2_theta = math::unit_to_tan2_theta(wo);
        if (std::isinf(tan2_theta)) {
            return 0.f;
        }
        auto alpha2 = 0.f
        + math::sqr(math::unit_to_cos_theta(wo) * alpha_u)
        + math::sqr(math::unit_to_sin_theta(wo) * alpha_v);
        return (math::sqrt(1.f + alpha2 * tan2_theta) - 1.f) / 2.f;
    }

    auto smith_mask(
        math::Vector<f32, 3> const& wo,
        f32 alpha_u,
        f32 alpha_v
    ) noexcept -> f32 {
        return 1.f / (1.f + lambda(-wo, alpha_u, alpha_v));
    }

    auto smith_shadow(
        math::Vector<f32, 3> const& wo,
        math::Vector<f32, 3> const& wi,
        f32 alpha_u,
        f32 alpha_v
    ) noexcept -> f32 {
        return 1.f / (1.f + lambda(-wo, alpha_u, alpha_v) + lambda(wi, alpha_u, alpha_v));
    }

    auto trowbridge_reitz(
        math::Vector<f32, 3> const& wm,
        f32 alpha_u,
        f32 alpha_v
    ) noexcept -> f32 {
        auto tan2_theta = math::unit_to_tan2_theta(wm);
        if (std::isinf(tan2_theta)) {
            return 0.f;
        }

        auto cos2_theta = math::unit_to_cos2_theta(wm);
        auto cos4_theta = math::sqr(cos2_theta);
        if (cos4_theta < math::epsilon<f32>) {
            return 0.f;
        }

        auto cos_phi = math::unit_to_cos_phi(wm);
        auto sin_phi = math::unit_to_sin_phi(wm);
        auto e = tan2_theta * (0.f
        + math::sqr(cos_phi / alpha_u)
        + math::sqr(sin_phi / alpha_v));

        return 1.f / (math::pi * alpha_u * alpha_v * cos4_theta * math::sqr(1.f + e));
    }

    auto visible_trowbridge_reitz(
        math::Vector<f32, 3> const& wo,
        math::Vector<f32, 3> const& wm,
        f32 alpha_u,
        f32 alpha_v
    ) noexcept -> f32 {
        return 1.f
        * trowbridge_reitz(wm, alpha_u, alpha_v)
        * smith_mask(-wo, alpha_u, alpha_v)
        * math::abs(math::dot(-wo, wm))
        / math::abs(math::unit_to_cos_theta(-wo));
    }

    auto torrance_sparrow(
        bool reflective, f32 pr, f32 pt,
        spectra::Stochastic_Spectrum const& F, f32 D, f32 G,
        math::Vector<f32, 3> const& wo,
        math::Vector<f32, 3> const& wi,
        math::Vector<f32, 3> const& wm,
        spectra::Stochastic_Spectrum const& eta,
        f32 alpha_u,
        f32 alpha_v
    ) noexcept -> std::optional<Interaction> {
        auto cos_theta_o = math::unit_to_cos_theta(-wo);
        auto cos_theta_i = math::unit_to_cos_theta(wi);
        auto cos_theta_om = math::dot(-wo, wm);
        auto cos_theta_im = math::dot(wi, wm);

        auto f = F;
        auto pdf = 1.f;
        auto denom = 0.f;
        if (reflective) {
            f = F * D * G / math::abs(4.f * cos_theta_o * cos_theta_i);
            pdf = visible_trowbridge_reitz(wo, wm, alpha_u, alpha_v) / (4.f * math::abs(cos_theta_om)) * pr / (pr + pt);
        } else {
            denom = math::sqr(cos_theta_im + cos_theta_om / eta.value[0]);
            f = (1.f - F) * D * G * math::abs(cos_theta_om * cos_theta_im / (denom * cos_theta_i * cos_theta_o)) / math::sqr(eta.value[0]);
            pdf = visible_trowbridge_reitz(wo, wm, alpha_u, alpha_v) * math::abs(cos_theta_im) / denom * pt / (pr + pt);
        }

        return Interaction{f, wi, pdf};
    }

}
