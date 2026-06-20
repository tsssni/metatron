implementing math;

namespace mtt::math {
    public struct Reservoir {
        public f32 p_hat = 0.f;
        public f32 w_sum = 0.f;
        public f32 M = 0.f;
        public f32 W = 0.f;

        [mutating] public bool operator()(Reservoir r, f32 u) {
            let replaced = u < guarded_div(r.w_sum, w_sum + r.w_sum);
            if (replaced) p_hat = r.p_hat;
            w_sum += r.w_sum;
            M += r.M;
            return replaced;
        }

        [mutating] public func shift(f32 J) {
            w_sum = p_hat * W * M * J;
        }

        [mutating] public func finalize() {
            W = guarded_div(w_sum, M * p_hat);
        }
    }
}
