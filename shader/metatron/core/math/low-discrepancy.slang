implementing math;

namespace mtt::math {
    public func radical_inverse(u32 x, i32 b) -> f32 {
        var reversed = 0u;
        let limit = (~0u - b) / b;
        var inv_b = 1u;

        // stop when next reversed will overflow
        while (x != 0 && reversed < limit) {
            let next = x / b;
            let digit = x - next * b;
            reversed = reversed * b + digit;
            inv_b *= b;
            x = next;
        }

        return min(reversed / f32(inv_b), 1.f - epsilon);
    }

    public func inverse_radical(u32 reversed, i32 b, i32 n) -> u32 {
        var idx = 0u;
        for (var i = 0; i < n; ++i) {
            let digit = reversed % b;
            reversed /= b;
            idx = idx * b + digit;
        }
        return idx;
    }

    public func owen_scrambled_radical_inverse(u32 x, i32 b, u32 hash) -> f32 {
        var reversed = 0u;
        let limit = (~0u - b) / b;
        var inv_b = 1u;

        // stop when precision of highest number in current bit not enough
        while (1.f - f32(b - 1) / inv_b < 1.f && reversed < limit) {
            let next = x / b;
            let digit = x - next * b;
            let digit_hash = u32(mix_bits(hash ^ reversed) & 0xffffffffu);
            reversed = reversed * b + biject_permute(digit, b, digit_hash);
            inv_b *= b;
            x = next;
        }

        return min(reversed / f32(inv_b), 1.f - epsilon);
    }

    public func fast_binary_owen_scramble(u32 x, u32 hash) -> u32 {
        x = bit_reverse(x);
        x ^= x * 0x3d20adea;
        x += hash;
        x *= (hash >> 16) | 1;
        x ^= x * 0x05526c56;
        x ^= x * 0x53a22864;
        return bit_reverse(x);
    }
}
