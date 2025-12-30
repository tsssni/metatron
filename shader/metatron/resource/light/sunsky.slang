implementing light;

namespace mtt::light {
    static i32 sunsky_num_lambda = 11;
    static Array<f32, 11> sunsky_lambda = {
        320.f, 360.f, 400.f, 440.f, 480.f, 520.f, 560.f, 600.f, 640.f, 680.f, 720.f
    };
    static f32 sunsky_step = 40.f;
    static i32 sky_num_params = 9;
    static i32 sun_num_ctls = 4;
    static i32 sun_num_segments = 45;
    static i32 sun_num_limb_params = 6;
    
    static i32 tgmm_num_segments = 30;
    static i32 tgmm_num_mixture = 5;
    static i32 tgmm_num_bilinear = 4;
    static i32 tgmm_num_gaussian = tgmm_num_bilinear * tgmm_num_mixture;

    public struct Sunsky_Light: Light {
        fv3 d;
        fm44 t;

        f32 turbidity;
        f32 albedo;
        f32 cos_sun;
        f32 phi_sun;
        f32 area;
        f32 w_sky;

        buf<f32> sky_params;
        buf<f32> sky_radiance;
        buf<f32> sun_radiance;
        buf<f32> sun_limb;
        buf<math::Truncated_Gaussian_Distribution> tgmm_phi_distr;
        buf<math::Truncated_Gaussian_Distribution> tgmm_theta_distr;
        math::Array_Distribution tgmm_distr;
        math::Cone_Distribution sun_distr;

        public Optional<Interaction> operator()(
            math::Ray r, fv4 lambda
        ) {
            var wi = normalize(r.d);
            let cos_theta = math::unit_to_cos_theta(wi);
            let sin_theta = math::unit_to_sin_theta(wi);
            let cos_gamma = dot(d, wi);

            let s = math::cartesian_to_unit_spherical(r.d);
            var theta = s.x; var phi = s.y;
            theta = clamp(
                theta < math::pi * 0.5f ? theta : math::pi - theta,
                0.f, math::pi * 0.5f - 1e-2f
            );
            wi = math::unit_spherical_to_cartesian({theta, phi});

            var L = fv4(0);
            for (var i = 0; i < 4; ++i) {
                L[i] = hosek(lambda[i], math::unit_to_cos_theta(wi), dot(d, wi));
                if (i == 0 && math::constant(lambda)) { L = L[0]; break; }
            }

            let tgmm_phi = phi + math::pi * 0.5f - phi_sun;
            var sun_pdf = 0.f;
            var sky_pdf = 0.f;
            for (var i = 0; cos_theta >= 0.f && i < tgmm_num_gaussian; ++i)
                sky_pdf += tgmm_phi_distr[i].pdf(tgmm_phi) * tgmm_theta_distr[i].pdf(theta) * tgmm_distr.pdf[i];
            sky_pdf = math::guarded_div(sky_pdf, sin(theta));
            sun_pdf = cos_gamma >= cos_sun ? sun_distr.pdf() : 0.f;
            let pdf = lerp(sun_pdf, sky_pdf, w_sky);

            return Interaction(
                L,
                r.d,
                r.o + r.d * 65504.f,
                65504.f,
                pdf,
            );
        }

        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> {
            var wi: fv3;
            if (u[0] < w_sky) {
                let idx = tgmm_distr.sample(u[0] / w_sky);
                let u_phi = math::guarded_div(
                    u[0] / w_sky - tgmm_distr.cdf[idx],
                    tgmm_distr.cdf[idx + 1] - tgmm_distr.cdf[idx]
                );
                let u_theta = u[1];

                // data fix sun to phi = pi / 2
                let tgmm_phi = tgmm_phi_distr[idx].sample(u_phi);
                let tgmm_theta = tgmm_theta_distr[idx].sample(u_theta);
                let phi = tgmm_phi + phi_sun - math::pi * 0.5f;
                let theta = clamp(tgmm_theta, 1e-2f, math::pi * 0.5f - 1e-2f);
                wi = math::unit_spherical_to_cartesian({theta, phi});
            } else {
                let u_sun = fv2((u[0] - w_sky) / (1.f - w_sky), u[1]);
                wi = normalize(mul(t, fv4(sun_distr.sample(u_sun), 0.f)).xyz);
            }
            return this({ctx.r.o, wi}, ctx.lambda);
        }

        public func flags() -> Flags {
            return Flags::inf;
        }

        func hosek(f32 lambda, f32 cos_theta, f32 cos_gamma) -> f32 {
            if (lambda > sunsky_lambda[sunsky_num_lambda - 1]) return 0.f;
            let splitted = split(lambda);
            let low = splitted._0;
            let high = splitted._1;
            let alpha = splitted._2;

            var L = lerp(
                hosek_sky(low, cos_theta, cos_gamma),
                hosek_sky(high, cos_theta, cos_gamma),
                alpha
            );

            if (cos_gamma >= cos_sun) {
                L += area
                * lerp(
                    hosek_sun(low, cos_theta),
                    hosek_sun(high, cos_theta),
                    alpha
                )
                * lerp(
                    hosek_limb(low, cos_gamma),
                    hosek_limb(high, cos_gamma),
                    alpha
                );
            }

            return L;
        }

        func hosek_sky(i32 idx, f32 cos_theta, f32 cos_gamma) -> f32 {
            let gamma = acos(cos_gamma);
            let A = sky_params[idx * sky_num_params + 0];
            let B = sky_params[idx * sky_num_params + 1];
            let C = sky_params[idx * sky_num_params + 2];
            let D = sky_params[idx * sky_num_params + 3];
            let E = sky_params[idx * sky_num_params + 4];
            let F = sky_params[idx * sky_num_params + 5];
            let G = sky_params[idx * sky_num_params + 6];
            let I = sky_params[idx * sky_num_params + 7];
            let H = sky_params[idx * sky_num_params + 8];

            struct {
                f32 operator()(f32 g, f32 cos_alpha) {
                    return math::guarded_div(
                        1.f + math::sqr(cos_alpha),
                        pow(1.f + math::sqr(g) - 2.f * g * cos_alpha, 1.5f)
                    );
                }
            } chi;

            let c0 = 1.f + A * exp(math::guarded_div(B, (cos_theta + 0.01f)));
            let c1 = 0.f
            + C + D * exp(E * gamma)
            + F * math::sqr(cos_gamma)
            + G * chi(H, cos_gamma)
            + I * math::sqrt(cos_theta);

            return c0 * c1 * sky_radiance[idx] / spectra::CIE_Y_integral;
        }

        func hosek_sun(i32 idx, f32 cos_theta) -> f32 {
            let eta = math::pi * 0.5f - acos(cos_theta);
            let segment = min(
                sun_num_segments - 1,
                i32(pow(eta / (math::pi * 0.5f), 1.f / 3.f) * sun_num_segments)
            );
            let x = eta - math::pi * 0.5f * pow(f32(segment) / f32(sun_num_segments), 3);
            var L = 0.f;
            for (var i = 0; i < sun_num_ctls; ++i)
                L += sun_radiance[
                    segment * sunsky_num_lambda * sun_num_ctls + idx * sun_num_ctls + i
                ] * pow(x, i);
            return L / spectra::CIE_Y_integral;
        }

        func hosek_limb(i32 idx, f32 cos_gamma) -> f32 {
            let sin_gamma_sqr = 1.f - math::sqr(cos_gamma);
            let cos_psi_sqr = 1.f - sin_gamma_sqr / (1.f - math::sqr(cos_sun));
            let cos_psi = math::sqrt(cos_psi_sqr);
            var l = 0.f;
            for (var i = 0; i < sun_num_limb_params; ++i)
                l += sun_limb[idx * sun_num_limb_params + i] * pow(cos_psi, i);
            return l;
        }

        func split(f32 lambda) -> Tuple<i32, i32, f32> {
            let norm = (lambda - sunsky_lambda[0]) / sunsky_step;
            let low = min(sunsky_num_lambda - 2, norm);
            let high = low + 1;
            let alpha = norm - low;
            return {low, high, alpha};
        }
    }
}
