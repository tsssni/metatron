implementing spectra;

namespace mtt::spectra {
    public struct Color_Space {
        public enum Spectrum_Type {
            albedo,
            unbounded,
            illuminant,
        };

        public fm33 from_XYZ;
        public fm33 to_XYZ;
        public Spectrum_Tag illuminant;
        public Transfer_Function_Tag transfer_function;

        public i32 table_res;
        public f32 illuminant_Y_integral;
        public buf<f32> scale;
        public buf<f32> table;
    }
}
