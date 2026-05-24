implementing color;

namespace mtt::color {
    public struct Color_Space {
        public enum Spectrum_Type {
            albedo,
            unbounded,
            illuminant,
        };

        public fm33 from_XYZ;
        public fm33 to_XYZ;
        public u32 illuminant;
        public Transfer_Function transfer_function;

        public i32 table_res;
        public f32 illuminant_Y_integral;
        public buf<f32> scale;
        public buf<f32> table;
    }
}
