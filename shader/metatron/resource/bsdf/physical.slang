implementing bsdf;

namespace mtt::bsdf {
    static i32 fresnel_num_samples = 65536;
    static i32 fresnel_length = 256;
    public static buf<f32> fresnel_reflectance_table = {};

    public struct Physical_Bsdf: Bsdf {
        fv4 reflectance = {2.f};
        fv4 fresnel_reflectance = {2.f};
        fv4 eta = {0.f};
        fv4 k = {0.f};
        f32 alpha_u = {0.f};
        f32 alpha_v = {0.f};

        bool lambertian = false;
        bool dieletric = false;
        bool conductive = false;
        bool plastic = false;

        public __init() {}
        public __init(
            fv4 r,
            fv4 e, fv4 ek,
            f32 au, f32 av
        ) {
            reflectance = r;
            eta = e;
            k = ek;
            alpha_u = au;
            alpha_v = av;

            let has_base = i32(all(reflectance == saturate(reflectance)));
            let has_surface = i32(any(eta != fv4(0.f)));
            let has_conductor = i32(any(k != fv4(0.f)));

            let bitmask = has_base | (has_surface << 1) | (has_conductor << 2);
            lambertian = bitmask == 0b001;
            dieletric = bitmask == 0b010;
            conductive = bitmask == 0b110;
            plastic = bitmask == 0b011;

            if (!plastic) fresnel_reflectance = fv4(0.f);
            else for (var i = 0; i < 4; ++i) {
                let idx = eta[i] / 3.f * fresnel_length;
                let low = i32(idx);
                let high = clamp(low + 1, 0, fresnel_num_samples);
                let alpha = idx - low;
                fresnel_reflectance[i] = lerp(fresnel_reflectance_table[low], fresnel_reflectance_table[high], alpha);
            }
        }

        public Optional<Interaction> operator()(fv3 wo, fv3 wi) {
            let flags = this.flags();
            if (false
            || flags & Flags::specular
            || abs(wo[1]) < math::epsilon
            || abs(wi[1]) < math::epsilon
            ) return {};

            let reflective = -wo[1] * wi[1] > 0.f;
            let forward = wi[1] > 0.f;
            var wm = normalize(reflective ? -wo + wi : -wo + wi * eta[0]);
            if (wm[1] < 0.f) wm *= -1.f;
            if (false
            || (!dieletric && (!reflective || !forward))
            || abs(wm[1]) < math::epsilon
            || dot(-wo, wm) < 0.f
            || dot((reflective ? 1.f : -1.f) * wi, wm) < 0.f) return {};

            if (lambertian) return Interaction(
                lambert(reflectance), wi,
                math::Cosine_Hemisphere_Distribution().pdf(math::unit_to_cos_theta(wi)),
            );

            let F = fresnel(dot(-wo, wm), eta, k);
            let D = trowbridge_reitz(wm, alpha_u, alpha_v);
            let G = smith_shadow(wo, wi, alpha_u, alpha_v);

            var pr = F[0];
            var pt = 1.f - F[0];
            pr *= float(bool(flags & Flags::reflective));
            pt *= float(bool(flags & Flags::transmissive));
            if (pr == 0.f && pt == 0.f) return {};

            let R_opt = torrance_sparrow(
                reflective, pr, pt,
                F, D, G,
                wo, wi, wm,
                eta, alpha_u, alpha_v
            );
            if (R_opt == none) return {};
            var R = R_opt.value;

            if (plastic) {
                let Fi = fresnel(math::unit_to_cos_theta(wi), eta, k);
                let Fo = fresnel(math::unit_to_cos_theta(-wo), eta, k);
                let pdf = math::Cosine_Hemisphere_Distribution().pdf(math::unit_to_cos_theta(wi));
                let internal = 1.f
                * (1.f - Fi) * (1.f - Fo) / (math::pi * math::sqr(eta))
                * (reflectance / (1.f - reflectance * fresnel_reflectance));
                R.f += internal;
                R.pdf *= Fo[0];
                R.pdf += (1.f - Fo[0]) * pdf;
            }
            return R;
        }

        public func sample(math::Context ctx, fv3 u) -> Optional<Interaction> {
            let wo = ctx.r.d;
            let flags = this.flags();
            let specular = bool(flags & Flags::specular);
            let Fo = (!specular && !plastic) ? fv4(0.f) :
            fresnel(math::unit_to_cos_theta(-ctx.r.d), eta, k);

            if (specular && (!plastic || u[0] < Fo[0])) {
                let wm = fv3(0.f, 1.f, 0.f);
                let cos_theta_o = math::unit_to_cos_theta(-wo);
                if (abs(wo[1]) < math::epsilon) return {};

                let pr = Fo[0] * float(bool(flags & Flags::reflective));
                let pt = (1.f - Fo[0]) * float(bool(flags & Flags::transmissive));
                if (pr == 0.f && pt == 0.f) return {};

                let reflective = u[0] < pr / (pr + pt);
                let wi = reflective ? reflect(wo, wm) : refract(wo, wm, eta[0]);
                let cos_theta_i = math::unit_to_cos_theta(wi);
                if (abs(wi[1]) < math::epsilon) return {};

                let pdf = (reflective ? pr : pt) / (pr + pt);
                let f = (reflective ? Fo : (1.f - Fo) / math::sqr(eta[0])) / abs(cos_theta_i);
                return Interaction(f, wi, pdf);
            } else if (dieletric || conductive || (plastic && u[0] < Fo[0])) {
                if (abs(wo[1]) < math::epsilon) return {};

                let wy = normalize(-wo * fv3(alpha_u, 1.f, alpha_v));
                let wx = wy[1] < 1.f - math::epsilon
                ? cross(wy, fv3(0.f, 1.f, 0.f))
                : fv3(1.f, 0.f, 0.f);
                let wz = cross(wx, wy);

                // use polar disk distribution to fetch more samples near center
                let distr = math::Polar_Disk_Distribution();
                var sample_p = distr.sample({u[1], u[2]});
                var sample_h = math::sqrt(1.f - math::sqr(sample_p[0]));
                sample_p[1] = (1.f + wy[1]) / 2.f * sample_p[1] + (1.f - wy[1]) * sample_h / 2.f;

                let sample_y = math::sqrt(1.f - dot(sample_p, sample_p));
                var wm = sample_p[0] * wx + sample_y * wy + sample_p[1] * wz;
                if (false
                || abs(wm[1]) < math::epsilon
                || dot(-wo, wm) < 0.f) return {};
                // normal transformation with inverse transposed matrix
                wm = normalize(wm * fv3(alpha_u, 1.f, alpha_v));

                let F = fresnel(dot(-wo, wm), eta, k);
                let D = trowbridge_reitz(wm, alpha_u, alpha_v);

                let pr = F[0] * float(bool(flags & Flags::reflective));
                let pt = (1.f - F[0]) * float(bool(flags & Flags::transmissive));
                if (pr == 0.f && pt == 0.f) return {};

                let reflective = (plastic || conductive) ? true : u[0] < pr / (pr + pt);
                let wi = reflective ? reflect(wo, wm) : refract(wo, wm, eta[0]);
                let G = smith_shadow(wo, wi, alpha_u, alpha_v);
                if (abs(wi[1]) < math::epsilon) return {};

                let R_opt = torrance_sparrow(
                    reflective, pr, pt,
                    F, D, G,
                    wo, wi, wm,
                    eta, alpha_u, alpha_v
                );
                if (R_opt == none) return {};
                var R = R_opt.value;
                R.pdf *= plastic ? Fo[0] : 1.f;
                return R;
            } else {
                let distr = math::Cosine_Hemisphere_Distribution();
                let wi = distr.sample({u[1], u[2]});
                var pdf = distr.pdf(wi[1]);
                if (abs(wi[1]) < math::epsilon) return {};

                if (lambertian) {
                    let f = lambert(reflectance);
                    return Interaction(f, wi, pdf);
                } else {
                    let Fi = fresnel(math::unit_to_cos_theta(wi), eta, k);
                    let f = 1.f
                    * (1.f - Fi) * (1.f - Fo) / (math::pi * math::sqr(eta))
                    * (reflectance / (1.f - reflectance * fresnel_reflectance));
                    pdf *= (1.f - Fo[0]);
                    return Interaction(f, wi, pdf);
                }
            }
        }

        public func flags() -> Flags {
            var flags = 0;
            if (lambertian || plastic || conductive) flags |= i32(Flags::reflective);
            else if (dieletric && all(eta == fv4(1.f))) flags |= i32(Flags::transmissive);
            else flags |= i32(Flags::transmissive | Flags::reflective);
            if (!lambertian && (alpha_u == 0.f || alpha_v == 0.f)) flags |= i32(Flags::specular);
            return Flags(flags);
        }
    }
}
