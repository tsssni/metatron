#pragma once
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/prime.hpp>
#include <metatron/core/math/hash.hpp>

namespace mtt::math {
    auto constexpr radical_inverse(u32 x, i32 b) noexcept -> f32 {
        auto reversed = 0u;
        auto limit = (~0u - b) / b;
        auto inv_b = 1u;

        while (x && reversed < limit) {
            auto next = x / b;
            auto digit = x - next * b;
            reversed = reversed * b + digit;
            inv_b *= b;
            x = next;
        }

        return min(reversed / f32(inv_b), 1.f - epsilon<f32>);
    }

    auto constexpr inverse_radical(u32 reversed, i32 b, i32 n) noexcept -> u32 {
        auto idx = 0u;
        for (auto i = 0; i < n; ++i) {
            auto digit = reversed % b;
            reversed /= b;
            idx = idx * b + digit;
        }
        return idx;
    }

    auto constexpr owen_scrambled_radical_inverse(u32 x, i32 b, u32 hash) noexcept -> f32 {
        auto reversed = 0u;
        auto limit = (~0u - b) / b;
        auto inv_b = 1u;

        while (1.f - f32(b - 1) / inv_b < 1.f && reversed < limit) {
            auto next = x / b;
            auto digit = x - next * b;
            auto digit_hash = mix_bits(hash ^ reversed);
            reversed = reversed * b + biject_permute(digit, b, digit_hash);
            inv_b *= b;
            x = next;
        }

        return min(reversed / f32(inv_b), 1.f - epsilon<f32>);
    }

    auto constexpr fast_binary_owen_scramble(u32 x, u32 hash) noexcept -> u32 {
        x = bit_reverse(x);
        x ^= x * 0x3d20adea;
        x += hash;
        x *= (hash >> 16) | 1;
        x ^= x * 0x05526c56;
        x ^= x * 0x53a22864;
        return bit_reverse(x);
    }
}
