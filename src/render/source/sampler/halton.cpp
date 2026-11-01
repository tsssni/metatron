#include <metatron/render/sampler/halton.hpp>
#include <metatron/core/math/low-discrepancy.hpp>
#include <metatron/core/math/number-theory.hpp>

namespace mtt::render {

Halton_Sampler::Halton_Sampler(uv2 scale_exponential) noexcept
: exponential(scale_exponential), scale({1u << scale_exponential[0], core::pow(3u, scale_exponential[1])}) {
    auto mulinv = uv2{
        core::multiplicative_inverse(scale[0], scale[1]),
        core::multiplicative_inverse(scale[1], scale[0])
    };
    stride = scale[0] * scale[1];
    scale_mulinv = scale * mulinv;
}

auto Halton_Sampler::start(Sampler_Context& ctx) const noexcept -> void {
    ctx.dim = core::clamp(ctx.dim, 2u, u32(core::primes.size()) - 1);

    // high num_exponetial bits of radical_inverse(halton_index) equals pixel % (base ^ num_exoinential),
    // so low num_exponetial bits of halton_index equals radical_inverse(pixel % (base ^ num_exoinetial))
    auto inverse_radical = [&](usize x, usize i) -> usize { return core::inverse_radical(x, core::primes[i], exponential[i]); };
    auto halton_low_digits = foreach(inverse_radical, core::mod(ctx.pixel, scale));

    // halton_index≡halton_low_digits[i](mod base^num_exponential[i])
    // use precomputed multiplicative inverse of scale to evaluate CRT
    // halton_index = chinese_remainder_theorem(halton_low_digits, scale);
    auto halton_idx = core::sum(halton_low_digits * scale_mulinv) % stride;

    // each sample has a LCM stride as we use core::primes as bases
    halton_idx += ctx.idx * stride;
    ctx.data[0] = halton_idx;
}

auto Halton_Sampler::generate_1d(Sampler_Context& ctx) const noexcept -> f32 {
    if (ctx.dim >= core::primes.size()) ctx.dim = 2;
    auto halton_idx = ctx.data[0];
    auto scrambled = core::owen_scrambled_radical_inverse(
        halton_idx, core::primes[ctx.dim], core::mix_bits(ctx.seed ^ ctx.dim)
    );
    ++ctx.dim;
    return scrambled;
}

auto Halton_Sampler::generate_2d(Sampler_Context& ctx) const noexcept -> fv2 {
    return {generate_1d(ctx), generate_1d(ctx)};
}

auto Halton_Sampler::generate_pixel_2d(Sampler_Context& ctx) const noexcept -> fv2 {
    // remove integer part by dividing scale
    auto halton_idx = ctx.data[0];
    return {
        core::radical_inverse(halton_idx >> exponential[0], core::primes[0]),
        core::radical_inverse(halton_idx / scale[1], core::primes[1])
    };
}

}
