implementing spectra;

namespace mtt::spectra {
    struct Color_Space {
        enum Spectrum_Type {
            albedo,
            unbounded,
            illuminant,
        };

        fm33 from_XYZ;
        fm33 to_XYZ;
        Spectrum_Tag illuminant;
        Transfer_Function_Tag transfer_function;

        i32 table_res;
        f32 illuminant_Y_integral;
        buf<f32> scale;
        buf<f32> table;
    }
}
