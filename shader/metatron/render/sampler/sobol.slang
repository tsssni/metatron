implementing sampler;

namespace mtt::sampler {
    static u32 num_sobol_dimensions = 2;
    static u32 sobol_matrix_size = 52;

    public struct Sobol_Sampler: Sampler {
        buf<u32> matrices = {};

        public func start(inout Context ctx) -> void {
            let log2_spp = u32(math::log2i(ctx.spp));
            let res = math::bit_ceil(u32(math::max(ctx.size)));
            let log4_spp = (log2_spp + 1) / 2;
            let base4_digits = math::log2i(res) + log4_spp;
            let morton_idx = (math::morton_encode(ctx.pixel) << log2_spp) | ctx.idx;

            ctx.data[0] = log2_spp;
            ctx.data[1] = base4_digits;
            ctx.data[2] = morton_idx;
        }

        public func generate_1d(inout Context ctx) -> f32 {
            let idx = permute_idx(ctx);
            ++ctx.dim;
            let key = Array<uv2, 1>({ctx.dim, ctx.seed});
            return sobol(idx, 0, u32(math::murmur_hash(key)));
        }

        public func generate_2d(inout Context ctx) -> fv2 {
            let idx = permute_idx(ctx);
            ctx.dim += 2;
            let key = Array<uv2, 1>({ctx.dim, ctx.seed});
            let bits = math::murmur_hash(key);
            return {
                sobol(idx, 0, u32(bits)),
                sobol(idx, 1, u32(bits >> 32)),
            };
        }

        public func generate_pixel_2d(inout Context ctx) -> fv2 {
            return generate_2d(ctx);
        }

        func permute_idx(Context ctx) -> u64 {
            static Array<bv4, 24> permutations = {
                {0, 1, 2, 3},
                {0, 1, 3, 2},
                {0, 2, 1, 3},
                {0, 2, 3, 1},
                {0, 3, 2, 1},
                {0, 3, 1, 2},
                {1, 0, 2, 3},
                {1, 0, 3, 2},
                {1, 2, 0, 3},
                {1, 2, 3, 0},
                {1, 3, 2, 0},
                {1, 3, 0, 2},
                {2, 1, 0, 3},
                {2, 1, 3, 0},
                {2, 0, 1, 3},
                {2, 0, 3, 1},
                {2, 3, 0, 1},
                {2, 3, 1, 0},
                {3, 1, 2, 0},
                {3, 1, 0, 2},
                {3, 2, 1, 0},
                {3, 2, 0, 1},
                {3, 0, 2, 1},
                {3, 0, 1, 2}
            };

            let log2_spp = ctx.data[0];
            let base4_digits = ctx.data[1];
            let morton_idx = ctx.data[2];

            var idx = 0ull;
            let last_digit = log2_spp & 1;
            for (var i = i32(base4_digits) - 1; i >= i32(last_digit); --i) {
                let shift = 2 * i - last_digit;
                let digit = (morton_idx >> shift) & 3;
                let higher_digits = morton_idx >> (shift + 2);
                let p = (math::mix_bits(higher_digits ^ (0x55555555u * ctx.dim)) >> 24) % 24;
                idx |= u64(permutations[p][digit]) << shift;
            }

            if (last_digit == 1) {
                int digit = morton_idx & 1;
                idx |= digit ^ (math::mix_bits((morton_idx >> 1) ^ (0x55555555u * ctx.dim)) & 1);
            }
            return idx;
        }

        func sobol(u64 idx, i32 dim, u32 hash) -> f32 {
            var x = 0u;
            for (var i = dim * sobol_matrix_size; idx != 0; idx >>= 1, ++i)
                if ((idx & 1) != 0) x ^= matrices[i];
            x = math::fast_binary_owen_scramble(x, hash);
            return min(x * 0x1p-32f, 1.f - math::epsilon);
        }
    }
}
