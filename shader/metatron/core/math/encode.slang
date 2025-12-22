implementing math;

namespace mtt::math {
    public func morton_encode(uv2 x) -> u32 {
        struct {
            u32 operator()(u32 x) {
                x = (x | (x << 16)) & 0x0000ffff;
                x = (x | (x << 8)) & 0x00ff00ff;
                x = (x | (x << 4)) & 0x0f0f0f0f;
                x = (x | (x << 2)) & 0x33333333;
                x = (x | (x << 1)) & 0x55555555;
                return x;
            }
        } spread_bits;
        return (spread_bits(x[1]) << 1) | spread_bits(x[0]);
    }
}
