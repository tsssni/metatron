implementing spectra;

namespace mtt::spectra {
    public struct Stochastic_Spectrum {
        public fv4 lambda = {};
        public fv4 value = {};
        public __init(fv4 lambda, fv4 value) {
            this.lambda = lambda;
            this.value = value;
        }

        public __init(f32 u, f32 v = 0) {
            for (var i = 0; i < 4; ++i) {
                let ui = fmod(u + f32(i) / 4.f, 1.f);
                lambda[i] = math::Spectrum_Distribution().sample(ui);
            }
            value = {v};
        }

        public f32 operator()(Spectrum s) {
            var pdf: fv4;
            for (var i = 0; i < 4; ++i)
                pdf[i] = math::Spectrum_Distribution().pdf(lambda[i]);
            return math::sum(value * (lambda & s) / pdf) / 4.f;
        }
    }
}
