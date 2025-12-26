implementing sampler;

namespace mtt::sampler {
    public struct Halton_Sampler: Sampler {
        iv2 pixel;
        iv2 exponential;
        iv2 scale;
        iv2 scale_mulinv;
        i32 idx;
        i32 stride;
        u32 seed;
        u32 dim;
        u32 halton_idx;

        [mutating] public func start(Context ctx) -> void {
            pixel = iv2(ctx.pixel);
            idx = ctx.idx;
            dim = clamp(ctx.dim, 2u, u32(math::primes.getCount()) - 1);
            seed = ctx.seed;

            var halton_low_digits: uv2;
            for (var i = 0; i < 2; ++i)
                halton_low_digits[i] = math::inverse_radical(
                    pixel[i] % scale[i], math::primes[i], exponential[i]
                );

            halton_idx = math::sum(halton_low_digits * scale_mulinv) % stride;
            halton_idx += idx * stride;
        }

        [mutating] public func generate_1d() -> f32 {

            if (dim >= math::primes.getCount()) dim = 2;
            let scrambled = math::owen_scrambled_radical_inverse(
                halton_idx, math::primes[dim], u32(math::mix_bits(seed ^ dim))
            );
            ++dim;
            return scrambled;
        }

        [mutating] public func generate_2d() -> fv2 {
            return {generate_1d(), generate_1d()};
        }

        [mutating] public func generate_pixel_2d() -> fv2 {
            return {
                math::radical_inverse(halton_idx >> exponential[0], math::primes[0]),
                math::radical_inverse(halton_idx / scale[1], math::primes[1])
            };
        }
    }
}
