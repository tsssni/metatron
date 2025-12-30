#pragma once
#include <cstring>

namespace mtt::math {
    // https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/math.h
    // https://github.com/mmp/pbrt-v4/blob/master/src/pbrt/util/hash.h

    template<typename... Args>
    auto murmur_hash(Args... args) -> u64 {
        auto constexpr n = (sizeof(Args) + ... + 0);
        auto key = std::array<byte, n>{};
        auto offset = 0ull;

        auto copy_arg = [&key, &offset](auto const& arg) {
            std::memcpy(key.data() + offset, &arg, sizeof(arg));
            offset += sizeof(arg);
        }; (copy_arg(args), ...);

        auto constexpr m = 0xc6a4a7935bd1e995ull;
        auto constexpr r = 47ull;
        auto seed = 0ull;
        auto h = seed ^ (n * m);
        auto* head = key.data();
        auto* end = head + 8 * (n / 8);

        while (head != end) {
            auto k = 0uz;
            std::memcpy(&k, head, sizeof(u64));
            head += 8;

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        switch (n & 7) {
        case 7:
            h ^= u64(head[6]) << 48;
        case 6:
            h ^= u64(head[5]) << 40;
        case 5:
            h ^= u64(head[4]) << 32;
        case 4:
            h ^= u64(head[3]) << 24;
        case 3:
            h ^= u64(head[2]) << 16;
        case 2:
            h ^= u64(head[1]) << 8;
        case 1:
            h ^= u64(head[0]);
            h *= m;
        default:;
        };

        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
    }

    auto inline constexpr mix_bits(u64 x) noexcept -> u64 {
        x ^= (x >> 31);
        x *= 0x7fb5d329728ea185ull;
        x ^= (x >> 27);
        x *= 0x81dadef4bc2dd44dull;
        x ^= (x >> 33);
        return x;
    }

    auto inline constexpr biject_permute(u32 i, u32 n, u32 seed) noexcept -> u32 {
        // set 1 for bits below highest 1
        auto w = n - 1;
        w |= w >> 1;
        w |= w >> 2;
        w |= w >> 4;
        w |= w >> 8;
        w |= w >> 16;

        do {
            i ^= seed;
            i *= 0xe170893d;
            i ^= seed >> 16;
            i ^= (i & w) >> 4;
            i ^= seed >> 8;
            i *= 0x0929eb3f;
            i ^= seed >> 23;
            i ^= (i & w) >> 1;
            i *= 1 | seed >> 27;
            i *= 0x6935fa69;
            i ^= (i & w) >> 11;
            i *= 0x74dcb303;
            i ^= (i & w) >> 2;
            i *= 0x9e501cc3;
            i ^= (i & w) >> 2;
            i *= 0xc860a3df;
            i &= w;
            i ^= i >> 5;
        } while (i >= n);

        return (i + seed) % n;
    }
}
