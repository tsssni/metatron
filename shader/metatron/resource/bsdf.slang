module bsdf;
__include bsdf.interface;
__include bsdf.physical;
import metatron.core;

namespace mtt::bsdf {
    public struct Interaction {
        public fv4 f;
        public fv3 wi;
        public f32 pdf;
        public bool degraded = false;
    }

    public enum Flags {
        reflective = 1 << 0,
        transmissive = 1 << 1,
        specular = 1 << 2,
        interface = 1 << 3,
    }

    public interface Bsdf {
        public Optional<Interaction> operator()(fv3 wo, fv3 wi);
        public func sample(math::Context ctx, fv3 u) -> Optional<Interaction>;
        public func flags() -> Flags;
    }

    public struct Bsdf_Union: Bsdf {
        Physical_Bsdf physical = {};
        Interface_Bsdf interface = {};
        u32 idx = 0xffffffffu;
        public __init() {}
        public __init(Physical_Bsdf p) { physical = p; idx = 0; }
        public __init(Interface_Bsdf i) { interface = i; idx = 1; }

        public Optional<Interaction> operator()(fv3 wo, fv3 wi) {
            if (idx == 0) return physical(wo, wi);
            else if (idx == 1) return interface(wo, wi);
            else return {};
        }

        public func sample(math::Context ctx, fv3 u) -> Optional<Interaction> {
            if (idx == 0) return physical.sample(ctx, u);
            else if (idx == 1) return interface.sample(ctx, u);
            else return {};
        }

        public func flags() -> Flags {
            if (idx == 0) return physical.flags();
            else if (idx == 1) return interface.flags();
            else return {};
        }
    }

    public func lambert(f32 reflectance) -> f32 {
        return reflectance / math::pi;
    }

    public func lambert(fv4 reflectance) -> fv4 {
        return reflectance / math::pi;
    }

    public func fresnel(f32 cos_theta_i, f32 eta) -> f32 {
        let sin2_theta_i = max(0.f, 1.f - cos_theta_i * cos_theta_i);
        let sin2_theta_t = math::guarded_div(sin2_theta_i, eta * eta);
        let cos_theta_t = math::sqrt(1.f - sin2_theta_t);

        let r_parl = math::guarded_div(
            eta * cos_theta_i - cos_theta_t,
            eta * cos_theta_i + cos_theta_t
        );
        let r_perp = math::guarded_div(
            cos_theta_i - eta * cos_theta_t,
            cos_theta_i + eta * cos_theta_t
        );
        return (math::sqr(r_parl) + math::sqr(r_perp)) / 2.f;
    }

    public func fresnel(f32 cos_theta_i, fc eta) -> f32 {
        let sin2_theta_i = max(0.f, 1.f - cos_theta_i * cos_theta_i);
        let sin2_theta_t = math::guarded_div(sin2_theta_i, eta * eta);
        let cos_theta_t = math::sqrt(1.f - sin2_theta_t);

        let conductive = eta.i > math::epsilon;
        if (!conductive && sin2_theta_t.r >= 1.f) return 1.f;

        let r_parl = math::guarded_div(
            eta * cos_theta_i - cos_theta_t,
            eta * cos_theta_i + cos_theta_t
        );
        let r_perp = math::guarded_div(
            cos_theta_i - eta * cos_theta_t,
            cos_theta_i + eta * cos_theta_t
        );
        return (math::norm(r_parl) + math::norm(r_perp)) / 2.f;
    }


    public func fresnel(f32 cos_theta_i, f32 eta, f32 k) -> f32 {
        cos_theta_i = clamp(cos_theta_i, -1.f, 1.f);

        if (k > math::epsilon) {
            let eta_k = fc(eta, k);
            return fresnel(cos_theta_i, eta_k);
        } else {
            return fresnel(cos_theta_i, eta);
        }
    }

    public func fresnel(f32 cos_theta_i, fv4 eta, fv4 k) -> fv4 {
        if (math::constant(eta) && math::constant(k)) return fv4(fresnel(cos_theta_i, eta[0], k[0]));
        var f: fv4;
        for (var i = 0; i < 4; ++i)
            f[i] = fresnel(cos_theta_i, eta[i], k[i]);
        return f;
    }

    public func lambda(fv3 wo, f32 alpha_u, f32 alpha_v) -> f32 {
        let tan2_theta = math::unit_to_tan2_theta(wo);
        if (isinf(tan2_theta)) return 0.f;
        let alpha2 = 0.f
        + math::sqr(math::unit_to_cos_theta(wo) * alpha_u)
        + math::sqr(math::unit_to_sin_theta(wo) * alpha_v);
        return (math::sqrt(1.f + alpha2 * tan2_theta) - 1.f) / 2.f;
    }

    public func smith_mask(fv3 wo, f32 alpha_u, f32 alpha_v) -> f32 {
        return 1.f / (1.f + lambda(-wo, alpha_u, alpha_v));
    }

    public func smith_shadow(fv3 wo, fv3 wi, f32 alpha_u, f32 alpha_v) -> f32 {
        return 1.f / (1.f + lambda(-wo, alpha_u, alpha_v) + lambda(wi, alpha_u, alpha_v));
    }

    public func trowbridge_reitz(fv3 wm, f32 alpha_u, f32 alpha_v) -> f32 {
        let tan2_theta = math::unit_to_tan2_theta(wm);
        if (isinf(tan2_theta)) return 0.f;

        let cos2_theta = math::unit_to_cos2_theta(wm);
        let cos4_theta = math::sqr(cos2_theta);
        if (cos4_theta < math::epsilon) return 0.f;

        let cos_phi = math::unit_to_cos_phi(wm);
        let sin_phi = math::unit_to_sin_phi(wm);
        let e = tan2_theta * (0.f
        + math::sqr(cos_phi / alpha_u)
        + math::sqr(sin_phi / alpha_v));

        return 1.f / (math::pi * alpha_u * alpha_v * cos4_theta * math::sqr(1.f + e));
    }

    public func visible_trowbridge_reitz(fv3 wo, fv3 wm, f32 alpha_u, f32 alpha_v) -> f32 {
        return 1.f
        * trowbridge_reitz(wm, alpha_u, alpha_v)
        * smith_mask(-wo, alpha_u, alpha_v)
        * abs(dot(-wo, wm))
        / abs(math::unit_to_cos_theta(-wo));
    }

    public func torrance_sparrow(
        bool reflective, f32 pr, f32 pt,
        fv4 F, f32 D, f32 G,
        fv3 wo, fv3 wi, fv3 wm,
        fv4 eta, f32 alpha_u, f32 alpha_v
    ) -> Optional<Interaction> {
        let cos_theta_o = math::unit_to_cos_theta(-wo);
        let cos_theta_i = math::unit_to_cos_theta(wi);
        let cos_theta_om = dot(-wo, wm);
        let cos_theta_im = dot(wi, wm);

        var f = F;
        var pdf = 1.f;
        var denom = 0.f;
        if (reflective) {
            f = F * D * G / abs(4.f * cos_theta_o * cos_theta_i);
            pdf = visible_trowbridge_reitz(wo, wm, alpha_u, alpha_v) / (4.f * abs(cos_theta_om)) * pr / (pr + pt);
        } else {
            denom = math::sqr(cos_theta_im + cos_theta_om / eta[0]);
            f = (1.f - F) * D * G * abs(cos_theta_om * cos_theta_im / (denom * cos_theta_i * cos_theta_o)) / math::sqr(eta[0]);
            pdf = visible_trowbridge_reitz(wo, wm, alpha_u, alpha_v) * abs(cos_theta_im) / denom * pt / (pr + pt);
        }

        if (pdf < math::epsilon) return {};
        return Interaction(f, wi, pdf);
    }
}

