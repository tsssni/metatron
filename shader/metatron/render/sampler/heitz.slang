implementing sampler;

namespace mtt::sampler {
    static u32 heitz_spp = 256;
    static u32 heitz_sobol_n = 256;
    static u32 heitz_sobol_d = 256;
    static u32 heitz_tile_size = 128;
    static u32 heitz_tile_d = 8;

    public struct Heitz_Sampler {
        buf<u32> sobol = {};
        buf<u32> ranking = {};
        buf<u32> scrambling = {};

        public func start(inout Context ctx) -> void {}

        func sample(uv2 pixel, u32 sample_idx, u32 dim, u32 seed) -> f32 {
            let i = pixel[0] & (heitz_tile_size - 1);
            let j = pixel[1] & (heitz_tile_size - 1);
            let batch = sample_idx / heitz_spp;
            let s = sample_idx & (heitz_spp - 1);
            let d_full = dim & (heitz_sobol_d - 1);
            let d_tile = dim & (heitz_tile_d - 1);

            let tile_pixel = (i + j * heitz_tile_size) * heitz_tile_d;
            let ranked = s ^ ranking[d_tile + tile_pixel];
            var value = sobol[d_full + ranked * heitz_sobol_d];
            value = value ^ scrambling[d_tile + tile_pixel];
            value = (value ^ math::mix_bits(batch ^ seed)) & 0xffu;
            return (0.5f + f32(value)) / 256.0f;
        }

        public func generate_1d(inout Context ctx) -> f32 {
            let v = sample(ctx.pixel, ctx.idx, ctx.dim, ctx.seed);
            ++ctx.dim;
            return v;
        }

        public func generate_2d(inout Context ctx) -> fv2 {
            let ux = sample(ctx.pixel, ctx.idx, ctx.dim, ctx.seed);
            let uy = sample(ctx.pixel, ctx.idx, ctx.dim + 1, ctx.seed);
            ctx.dim += 2;
            return {ux, uy};
        }

        public func generate_pixel_2d(inout Context ctx) -> fv2 {
            return generate_2d(ctx);
        }
    }
}
