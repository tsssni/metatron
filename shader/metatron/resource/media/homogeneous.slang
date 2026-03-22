implementing media;

namespace mtt::media {
    public struct Homogeneous_Medium {
        Phase phase = {};
        spectra::Spectrum_Tag sigma_a = {};
        spectra::Spectrum_Tag sigma_s = {};
        spectra::Spectrum_Tag sigma_e = {};

        public struct Iterator {
            Homogeneous_Medium medium = {};
            math::Ray r = {};
            fv4 lambda = {};
            f32 t_max = 0.f;

            __init() {}
            __init(Homogeneous_Medium medium, math::Ray r, fv4 lambda, f32 t_max) {
                this.medium = medium;
                this.r = r;
                this.lambda = lambda;
                this.t_max = t_max;
            }

            [mutating] public func march(f32 u) -> Optional<Interaction> {
                let sigma_a = (lambda & medium.sigma_a);
                let sigma_s = (lambda & medium.sigma_s);
                let sigma_t = sigma_a + sigma_s;
                let sigma_maj = sigma_t;
                let sigma_n = fv4(0.f);

                let distr = math::Exponential_Distribution(sigma_t[0]);
                let t_u = distr.sample(u);
                let t = min(t_u, t_max);
                let pdf = t < t_max ? distr.pdf(t) : distr.pdf(t) / sigma_t[0];

                let sigma_e = (lambda & medium.sigma_e);
                let transmittance = exp(-sigma_maj * t);

                return Interaction(
                    medium.phase.to_phase(),
                    r.o + r.d * t,
                    t,
                    transmittance,
                    sigma_a,
                    sigma_s,
                    sigma_n,
                    sigma_maj,
                    sigma_e
                );
            }
        }

        public func begin(math::Context ctx, f32 t_max) -> Iterator {
            return Iterator(this, ctx.r, ctx.lambda, t_max);
        }
    }
}
