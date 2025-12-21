implementing spectra;

namespace mtt::spectra {
    public struct Blackbody_Spectrum: Spectrum {
        f32 T;

        public f32 operator()(f32 lambda) {
            if (T <= 0.f) return 0.f;

            static f32 c = 299792458.f;
            static f32 h = 6.62606957e-34f;
            static f32 kb = 1.3806488e-23f;

            let l = lambda * 1e-9f;
            let L = (2.f * h * c * c) / (pow(l, 5) * (exp((h * c) / (l * kb * T)) - 1.f));
            return isnan(L) ? 0.f : L;
        }
    };
}
