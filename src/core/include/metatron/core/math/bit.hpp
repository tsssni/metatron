#pragma once
#include <concepts>

namespace mtt::math {
    template<typename T>
    requires std::unsigned_integral<T>
    auto bit_reverse(T x) -> T {
        if constexpr (std::is_same_v<T, u32>) {
            x = (x << 16) | (x >> 16);
            x = ((x & 0x00ff00ff) << 8) | ((x & 0xff00ff00) >> 8);
            x = ((x & 0x0f0f0f0f) << 4) | ((x & 0xf0f0f0f0) >> 4);
            x = ((x & 0x33333333) << 2) | ((x & 0xcccccccc) >> 2);
            x = ((x & 0x55555555) << 1) | ((x & 0xaaaaaaaa) >> 1);
            return x;
        } else if constexpr (std::is_same_v<T, u64>) {
            auto n0 = u64(bit_reverse(u32(x)));
            auto n1 = u64(bit_reverse(u32(x >> 32)));
            return (n0 << 32) | n1;
        }
    }

    auto inline align(usize size, usize alignment) noexcept -> usize {
        if (!std::has_single_bit(alignment)) return 0;
        return (size + alignment - 1) & ~(alignment - 1);
    }
}
