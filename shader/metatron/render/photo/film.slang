implementing photo;
import metatron.core;

namespace mtt::photo {
    public struct Fixel {
        public uint2 pixel;
        public float2 position;
        public float2 dxdy;
        public float weight;
    }

    public struct Film {
        public uint spp;
        public uint depth;
        public float2 film_size;
        public float2 dxdy;
        public uint color_space;
        public uint r;
        public uint g;
        public uint b;

        public Fixel operator()(
            uint filter,
            uint2 pixel,
            uint2 size,
            float2 u
        ) {
            // let f_intr = *filter->sample(u);
            let pixel_position = float2(pixel) + 0.5; // + f_intr.p;
            let uv = pixel_position / size;
            let film_position = (uv - 0.5) * float2(-1.0, 1.0) * film_size;
            let dxdy = float2(select((film_position + dxdy - film_size / 2.f) < 0, 1, -1) * dxdy);

            var fixel: Fixel;
            fixel.pixel = pixel;
            fixel.position = film_position;
            fixel.dxdy = dxdy;
            fixel.weight = 1.0;
            return fixel;
        }
    };
}
