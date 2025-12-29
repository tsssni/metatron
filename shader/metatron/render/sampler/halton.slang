implementing sampler;

namespace mtt::sampler {
    public struct Halton_Sampler: Sampler {
        iv2 exponential = 0;
        iv2 scale = 0;
        iv2 scale_mulinv = 0;
        i32 stride = 0;

        public func start(inout Context ctx) -> void {
            ctx.dim = clamp(ctx.dim, 2u, u32(math::primes.getCount()) - 1);

            var halton_low_digits: uv2;
            for (var i = 0; i < 2; ++i)
                halton_low_digits[i] = math::inverse_radical(
                    ctx.pixel[i] % scale[i], math::primes[i], exponential[i]
                );

            var halton_idx = math::sum(halton_low_digits * scale_mulinv) % stride;
            halton_idx += ctx.idx * stride;
            ctx.data[0] = halton_idx;
        }

        public func generate_1d(inout Context ctx) -> f32 {

            if (ctx.dim >= math::primes.getCount()) ctx.dim = 2;
            let halton_idx = ctx.data[0];
            let scrambled = math::owen_scrambled_radical_inverse(
                halton_idx, math::primes[ctx.dim], u32(math::mix_bits(ctx.seed ^ ctx.dim))
            );
            ++ctx.dim;
            return scrambled;
        }

        public func generate_2d(inout Context ctx) -> fv2 {
            return {generate_1d(ctx), generate_1d(ctx)};
        }

        public func generate_pixel_2d(inout Context ctx) -> fv2 {
            let halton_idx = ctx.data[0];
            return {
                math::radical_inverse(halton_idx >> exponential[0], math::primes[0]),
                math::radical_inverse(halton_idx / scale[1], math::primes[1])
            };
        }
    }
}
