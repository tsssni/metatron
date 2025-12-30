#include <metatron/render/sampler/halton.hpp>
#include <metatron/core/math/prime.hpp>
#include <metatron/core/math/low-discrepancy.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/number-theory.hpp>

namespace mtt::sampler {
    Halton_Sampler::Halton_Sampler(cref<Descriptor> desc) noexcept:
    exponential(desc.scale_exponential),
    scale({1u << desc.scale_exponential[0], math::pow(3u, desc.scale_exponential[1])}) {
        stride = scale[0] * scale[1];
        scale_mulinv = scale * uv2{
            math::multiplicative_inverse(scale[0], scale[1]),
            math::multiplicative_inverse(scale[1], scale[0])
        };
    }

    auto Halton_Sampler::start(ref<Context> ctx) const noexcept -> void {
        ctx.dim = math::clamp(ctx.dim, 2u, u32(math::primes.size()) - 1);

        // high num_exponetial bits of radical_inverse(halton_index) equals pixel % (base ^ num_exoinential),
        // so low num_exponetial bits of halton_index equals radical_inverse(pixel % (base ^ num_exoinetial))
        auto halton_low_digits = foreach([&](usize x, usize i) -> usize {
            return math::inverse_radical(x, math::primes[i], exponential[i]);
        }, math::mod(ctx.pixel, scale));

        // halton_index≡halton_low_digits[i](mod base^num_exponential[i])
        // use precomputed multiplicative inverse of scale to evaluate CRT
        // halton_index = chinese_remainder_theorem(halton_low_digits, scale);
        auto halton_idx = math::sum(halton_low_digits * scale_mulinv) % stride;

        // each sample has a LCM stride as we use math::primes as bases
        halton_idx += ctx.idx * stride;
        ctx.data[0] = halton_idx;
    }

    auto Halton_Sampler::generate_1d(ref<Context> ctx) const noexcept -> f32 {
        if (ctx.dim >= math::primes.size()) ctx.dim = 2;
        auto halton_idx = ctx.data[0];
        auto scrambled = math::owen_scrambled_radical_inverse(
            halton_idx, math::primes[ctx.dim], math::mix_bits(ctx.seed ^ ctx.dim)
        );
        ++ctx.dim;
        return scrambled;
    }

    auto Halton_Sampler::generate_2d(ref<Context> ctx) const noexcept -> fv2 {
        return {generate_1d(ctx), generate_1d(ctx)};
    }

    auto Halton_Sampler::generate_pixel_2d(ref<Context> ctx) const noexcept -> fv2 {
        // remove integer part by dividing scale
        auto halton_idx = ctx.data[0];
        return {
            math::radical_inverse(halton_idx >> exponential[0], math::primes[0]),
            math::radical_inverse(halton_idx / scale[1], math::primes[1])
        };
    }
}
