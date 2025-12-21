implementing spectra;

namespace mtt::spectra {
    typedef Color_Space::Spectrum_Type Spectrum_Type;

    public struct Rgb_Spectrum: Spectrum {
        fv3 c = {};
        f32 s = {};
        Spectrum_Tag illuminant = {};

        public __init(
            fv3 c,
            Color_Space::Spectrum_Type type,
            tag<Color_Space> color_space,
        ) {
            var rgb = c;
            let cs = color_space.get();
            illuminant = type == Spectrum_Type::illuminant
            ? cs.illuminant : Spectrum_Tag();

            s = 1.f;
            switch (type) {
            case Spectrum_Type::albedo:
                s = 1.f;
                break;
            case Spectrum_Type::unbounded:
                s = 2.f;
                break;
            case Spectrum_Type::illuminant:
                s = 2.f * max(rgb[0], max(rgb[1], rgb[2]));
                break;
            }
            rgb = rgb / s;
            s /= type == Spectrum_Type::illuminant
            ? cs.illuminant_Y_integral : 1.f;

            if (rgb[0] == rgb[1] && rgb[1] == rgb[2]) {
                c = fv3(
                    (rgb[0] - 0.5f) / math::sqrt(rgb[0] * (1.f - rgb[0])),
                    0.f, 0.f,
                );
                return;
            }

            let maxc = math::maxi(rgb);
            let z = rgb[maxc];
            let x = rgb[(maxc + 1) % 3] * (cs.table_res - 1) / z;
            let y = rgb[(maxc + 2) % 3] * (cs.table_res - 1) / z;

            var bound = 0;
            for (; bound < cs.scale.size() && cs.scale[bound] < z; ++bound);
            let xi = min((i32)x, cs.table_res - 2);
            let yi = min((i32)y, cs.table_res - 2);
            let zi = clamp(bound - 1, 0, cs.table_res - 2);

            let dx = x - xi;
            let dy = y - yi;
            let dz = (z - cs.scale[zi]) / (cs.scale[zi + 1] - cs.scale[zi]);

            for (var i = 0; i < 3; ++i) {
                struct {
                    f32 operator()(i32 dx, i32 dy, i32 dz) {
                        return cs.table[0uz
                        + maxc      * math::pow(cs.table_res, 3) * 3
                        + (zi + dz) * math::pow(cs.table_res, 2) * 3
                        + (yi + dy) * math::pow(cs.table_res, 1) * 3
                        + (xi + dx) * 3 + i
                        ];
                    }
                } co;

                c[2 - i] = lerp(
                    lerp(
                        lerp(co(0, 0, 0), co(1, 0, 0), dx),
                        lerp(co(0, 1, 0), co(1, 1, 0), dx),
                        dy
                    ),
                    lerp(
                        lerp(co(0, 0, 1), co(1, 0, 1), dx),
                        lerp(co(0, 1, 1), co(1, 1, 1), dx),
                        dy
                    ),
                    dz
                );
            }
        }

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
