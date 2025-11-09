#include <metatron/render/sampler/halton.hpp>
#include <metatron/core/math/prime.hpp>
#include <metatron/core/math/low-discrepancy.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/number-theory.hpp>

namespace mtt::sampler {
    Halton_Sampler::Halton_Sampler(
        math::Vector<i32, 2> const& scale_exponential
    ) noexcept:
    exponential(scale_exponential),
    scale({1 << scale_exponential[0], math::pow(3, scale_exponential[1])}) {
        stride = scale[0] * scale[1];
        scale_mulinv = scale * math::Vector<usize, 2>{
            math::multiplicative_inverse(scale[0], scale[1]),
            math::multiplicative_inverse(scale[1], scale[0])
        };
    }

    auto Halton_Sampler::start(
        math::Vector<usize, 2> const& pixel,
        usize idx, usize dim, usize seed
    ) noexcept -> void {
        this->pixel = pixel;
        this->idx = idx;
        this->dim = math::clamp(dim, 2uz, math::primes.size() - 1uz);
        this->seed = seed;

        // high num_exponetial bits of radical_inverse(halton_index) equals pixel % (base ^ num_exoinential),
        // so low num_exponetial bits of halton_index equals radical_inverse(pixel % (base ^ num_exoinetial))
        auto halton_low_digits = foreach([&](usize x, usize i) -> usize {
            return math::inverse_radical(x, math::primes[i], exponential[i]);
        }, math::mod(math::Vector<i32, 2>{pixel}, scale));

        // halton_index≡halton_low_digits[i](mod base^num_exponential[i])
        // use precomputed multiplicative inverse of scale to evaluate CRT
        // halton_index = chinese_remainder_theorem(halton_low_digits, scale);
        halton_idx = sum(halton_low_digits * scale_mulinv) % stride;

        // each sample has a LCM stride as we use math::primes as bases
        halton_idx += idx * stride;
    }

    auto Halton_Sampler::generate_1d() noexcept -> f32 {
        if (dim >= math::primes.size()) dim = 2uz;
        auto scrambled = math::owen_scrambled_radical_inverse(
            halton_idx, math::primes[dim], math::mix_bits(seed ^ dim)
        );
        ++dim;
        return scrambled;
    }

    auto Halton_Sampler::generate_2d() noexcept -> math::Vector<f32, 2> {
        return {generate_1d(), generate_1d()};
    }

    auto Halton_Sampler::generate_pixel_2d() noexcept -> math::Vector<f32, 2> {
        // remove integer part by dividing scale
        return {
            math::radical_inverse(halton_idx >> exponential[0], math::primes[0]),
            math::radical_inverse(halton_idx / scale[1], math::primes[1])
        };
    }
}
