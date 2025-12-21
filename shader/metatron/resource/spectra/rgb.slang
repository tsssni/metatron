implementing spectra;

namespace mtt::spectra {
    public struct Rgb_Spectrum: Spectrum {
        fv3 c;
        f32 s;
        Spectrum_Tag illuminant;

        public f32 operator()(f32 lambda) {
            return s
            * sigmoid(math::polynomial(lambda, c))
            * (!illuminant.empty() ? (illuminant.get())(lambda) : 1.f);
        }

        func sigmoid(f32 x) -> f32 {
            if (isinf(x)) return x < 0.f ? 0.f : 1.f;
            return 0.5f + x / (2.f * math::sqrt(1.f + math::sqr(x)));
        }
    }
}
