implementing math;
import metatron.core.stl;

namespace mtt::math {
    public struct Sequential_Distribution {
        buf<f32> rows;
        buf<f32> cdf;

        i32 dim;
        f32 low;
        f32 high;
        f32 delta;
        f32 integral;

        public func sample(f32 u) -> f32 {
            var idx = 1;
            for (; idx < dim && cdf[idx] <= u; ++idx);
            idx--;
            let du = guarded_div(u - cdf[idx], cdf[idx + 1] - cdf[idx]);
            return lerp(low, high, (f32(idx) + du) / f32(dim));
        }

        public func pdf(f32 p) -> f32 {
            let idx = clamp(
                i32((p - low) / delta),
                0, dim - 1
            );
            return (cdf[idx + 1] - cdf[idx]) / delta;
        }
    }

    public struct Planar_Distribution {
        buf<f32> rows;
        buf<f32> row_cdf;
        buf<f32> cols;
        buf<f32> col_cdf;
        f32 integral;

        iv2 dim;
        fv2 low = {};
        fv2 high = {};
        fv2 delta = {};

        public func sample(fv2 u) -> fv2 {
            var ci = 1;
            for (; ci < dim[0] && col_cdf[ci] <= u[0]; ++ci);
            ci--;
            let cu = guarded_div(u[0] - col_cdf[ci], col_cdf[ci + 1] - col_cdf[ci]);
            let cp = lerp(low[0], high[0], (f32(ci) + cu) / f32(dim[0]));

            var offset = ci * (dim[1] + 1);
            var ri = 1;
            for (; ri < dim[1] && row_cdf[offset + ri] <= u[1]; ++ri);
            ri--; offset += ri;
            let ru = guarded_div(u[1] - row_cdf[offset], row_cdf[offset + 1] - row_cdf[offset]);
            let rp = lerp(low[1], high[1], (f32(ri) + ru) / f32(dim[1]));

            return {cp, rp};
        }

        public func pdf(fv2 p) -> f32 {
            let ci = clamp(
                i32((p[0] - low[0]) / delta[0]),
                0, dim[0] - 1
            );
            let cp = (col_cdf[ci + 1] - col_cdf[ci]) / delta[0];

            var offset = ci * (dim[1] + 1);
            var ri = clamp(
                i32((p[1] - low[1]) / delta[1]),
                0, dim[1] - 1
            );
            offset += ri;
            let rp = (row_cdf[offset + 1] - row_cdf[offset]) / delta[1];

            return cp * rp;
        }
    }
}
