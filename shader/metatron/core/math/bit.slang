implementing math;

namespace mtt::math {
    func bit_reverse(u32 x) -> u32 {
        x = (x << 16) | (x >> 16);
        x = ((x & 0x00ff00ff) << 8) | ((x & 0xff00ff00) >> 8);
        x = ((x & 0x0f0f0f0f) << 4) | ((x & 0xf0f0f0f0) >> 4);
        x = ((x & 0x33333333) << 2) | ((x & 0xcccccccc) >> 2);
        x = ((x & 0x55555555) << 1) | ((x & 0xaaaaaaaa) >> 1);
        return x;
    }

    func bit_reverse(u64 x) -> u64 {
        let n0 = u64(bit_reverse(u32(x)));
        let n1 = u64(bit_reverse(u32(x >> 32)));
        return (n0 << 32) | n1;
    }

    func align(u32 size, u32 alignment) -> u32 {
        if (countbits(alignment) != 1) return 0;
        return (size + alignment - 1) & ~(alignment - 1);
    }
}
