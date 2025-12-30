implementing spectra;

namespace mtt::spectra {
    public struct Constant_Spectrum: Spectrum {
        f32 x;
        public f32 operator()(f32 lambda) { return x; }
    };
}
