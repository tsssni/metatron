implementing math;

namespace mtt::math {
    public struct Discrete_Distribution<let n: u32> {
        public Array<f32, n> pdf;
        public Array<f32, n + 1> cdf;

        public __init() {}
        public __init(Array<f32, n> ws) {
            pdf = ws;
            cdf[0] = 0.f;
            for (var i = 1; i <= n; ++i)
                cdf[i] = cdf[i - 1] + pdf[i - 1];
            for (var i = 0; i < n; ++i)
                pdf[i] /= cdf[n];
        }

        public func sample(f32 u) -> u32 {
            var i = 0;
            for (; i < n && u > cdf[i + 1]; ++i);
            return i;
        }
    }

    public struct Array_Distribution {
        public buf<f32> pdf;
        public buf<f32> cdf;

        public func sample(f32 u) -> u32 {
            var i = 0u;
            for (; i < pdf.size() && u > cdf[i + 1]; ++i);
            return i;
        }
    }
}
