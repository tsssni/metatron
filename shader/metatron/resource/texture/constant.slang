implementing texture;

namespace mtt::texture {
    struct Constant_Spectrum_Texture: Spectrum_Texture {
        spectra::Spectrum_Tag x;
        fv4 operator()(Coordinate coord, fv4 spec) {
            return spec & x;
        }
        func sample(math::Context ctx, fv2 u) -> fv2 { return u; }
        func pdf(fv2 uv) -> f32 { return 1.f; }
    }

    struct Constant_Vector_Texture: Vector_Texture {
        fv4 x;
        fv4 operator()(Coordinate coord) {
            return x;
        }
        func sample(math::Context ctx, fv2 u) -> fv2 { return u; }
        func pdf(fv2 uv) -> f32 { return 1.f; }
    }
}
