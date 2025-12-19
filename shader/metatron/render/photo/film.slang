implementing photo;
import metatron.core;

namespace mtt::photo {
    public struct Fixel {
        public uv2 pixel;
        public fv2 position;
        public fv2 dxdy;
        public f32 weight;
    }

    public struct Film {
        public u32 spp;
        public u32 depth;
        public fv2 film_size;
        public fv2 dxdy;
        public u32 color_space;
        public u32 r;
        public u32 g;
        public u32 b;

        public Fixel operator()(
            u32 filter,
            uv2 pixel,
            uv2 size,
            fv2 u
        ) {
            // let f_intr = *filter->sample(u);
            let pixel_position = fv2(pixel) + 0.5; // + f_intr.p;
            let uv = pixel_position / size;
            let film_position = (uv - 0.5) * fv2(-1.0, 1.0) * film_size;
            let dxdy = fv2(select((film_position + dxdy - film_size / 2.f) < 0, 1, -1) * dxdy);

            var fixel: Fixel;
            fixel.pixel = pixel;
            fixel.position = film_position;
            fixel.dxdy = dxdy;
            fixel.weight = 1.0;
            return fixel;
        }
    };
}
