implementing photo;

namespace mtt::photo {
    public struct Fixel {
        public uv2 pixel;
        public fv2 position;
        public fv2 dxdy;
        public f32 weight;

        public fv3 operator()(Ptr<Film> film, spectra::Stochastic_Spectrum spectrum) {
            let xyz = fv3(
                spectrum(film.r),
                spectrum(film.g),
                spectrum(film.b),
            );
            return mul(film.color_space.get().from_XYZ, xyz);
        }
    }

    public struct Film {
        public u32 spp;
        public u32 depth;
        public u32 stride;
        public fv2 film_size;
        public fv2 dxdy;
        public color::proxy::Color_Space color_space;
        public spectra::Spectrum r;
        public spectra::Spectrum g;
        public spectra::Spectrum b;

        public Fixel operator()(
            filter::Filter filter,
            uv2 pixel,
            uv2 size,
            fv2 u
        ) {
            let f_opt = filter.sample(u);
            if (f_opt == none) return {};
            let f_intr = f_opt.value;
            let pixel_position = fv2(pixel) + 0.5f + f_intr.p;
            let uv = pixel_position / size;
            let film_position = (uv - 0.5) * fv2(-1.0, 1.0) * film_size;
            let dxdy = fv2(select((film_position + dxdy - film_size / 2.f) < 0, 1, -1) * dxdy);

            var fixel: Fixel;
            fixel.pixel = pixel;
            fixel.position = film_position;
            fixel.dxdy = dxdy;
            fixel.weight = math::guarded_div(f_intr.weight, f_intr.pdf);
            return fixel;
        }
    };
}
