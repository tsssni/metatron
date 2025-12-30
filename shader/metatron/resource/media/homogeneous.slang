implementing media;

namespace mtt::media {
    public struct Homogeneous_Medium: Medium {
        Phase phase;
        spectra::Spectrum_Tag sigma_a;
        spectra::Spectrum_Tag sigma_s;
        spectra::Spectrum_Tag sigma_e;

        public func sample(math::Context ctx, f32 t_max, f32 u) -> Optional<Interaction> {
            let sigma_a = (ctx.lambda & this.sigma_a);
            let sigma_s = (ctx.lambda & this.sigma_s);
            let sigma_t = sigma_a + sigma_s;
            let sigma_maj = sigma_t;
            let sigma_n = fv4(0.f);

            let distr = math::Exponential_Distribution(sigma_t[0]);
            let t_u = distr.sample(u);
            let t = min(t_u, t_max);
            let pdf = t < t_max ? distr.pdf(t) : distr.pdf(t) / sigma_t[0];

            let sigma_e = (ctx.lambda & this.sigma_e);
            let transmittance = exp(-sigma_maj * t);

            return Interaction(
                phase.to_phase(),
                ctx.r.o + ctx.r.d * t,
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
}
