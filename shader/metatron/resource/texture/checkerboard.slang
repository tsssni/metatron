implementing texture;

namespace mtt::texture {
    struct Checkerboard_Texture: Spectrum_Texture {
        spectra::Spectrum_Tag x;
        spectra::Spectrum_Tag y;
        uv2 uv_scale;

        f32 w_x;
        f32 w_y;

        fv4 operator()(Coordinate coord, fv4 spec) {
            let uv = uv2(coord.uv * uv_scale);
            let u = uv.x; let v = uv.y;
            let z = ((u + v) % 2 == 0) ? x : y;
            return spec & z.get();
        }

        func sample(math::Context ctx, fv2 u) -> fv2 {
            var uv: fv2;
            var i = 0;
            if (u[0] < w_x) {
                uv = fv2(u[0] / w_x, u[1]);
                i = 0;
            } else {
                uv = fv2((1.f - u[0]) / w_y, u[1]);
                i = 1;
            }

            uv *= uv_scale;
            if (u32(math::sum(floor(uv))) % 2 != i)
                uv[0] = fmod(uv[0] + 1.f, f32(uv_scale[0]));
            return uv;
        }

        func pdf(fv2 uv) -> f32 {
            let uv = uv2(uv * uv_scale);
            return math::sum(uv) % 2 == 0
            ? w_x / math::prod(uv_scale)
            : w_y / math::prod(uv_scale);
        }
    }
}
