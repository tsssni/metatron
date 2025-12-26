implementing math;

namespace mtt::math {
    public func murmur_hash<let n: u32>(Array<u64, n> key, u64 seed) -> u64 {
        let m = 0xc6a4a7935bd1e995ull;
        let r = 47ull;
        var h = seed ^ (n * m);

        for (var i = 0; i < n; ++i) {
            var k = key[i];

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
    }


    public func murmur_hash<let n: u32>(Array<uv2, n> key) -> u64 {
        var buffer: Array<u64, n>;
        for (var i = 0; i < n; ++i)
            buffer[i] = u64(key[i].x) | (u64(key[i].y) << 32);
        return murmur_hash(buffer, 0);
    }

    public func mix_bits(u64 x) -> u64 {
        x ^= (x >> 31);
        x *= 0x7fb5d329728ea185ull;
        x ^= (x >> 27);
        x *= 0x81dadef4bc2dd44dull;
        x ^= (x >> 33);
        return x;
    }

    public func biject_permute(u32 i, u32 n, u32 seed) -> u32 {
        // set 1 for bits below highest 1
        var w = n - 1;
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
