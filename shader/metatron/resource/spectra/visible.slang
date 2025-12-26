implementing spectra;

namespace mtt::spectra {
    public static u32 visible_range = 471;

    public struct Visible_Spectrum: Spectrum {
        Array<f32, 471> storage;

        public f32 operator()(f32 lambda) {
            let idx = clamp(i32(round(lambda) - visible_lambda[0]), 0, visible_range - 1);
            return storage[idx];
        }
    }
}
