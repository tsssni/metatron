#pragma once
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::math {
    struct Reservoir final {
        f32 p_hat = 0;
        f32 w_sum = 0;
        f32 M = 0;
        f32 W = 0;

        auto merge(cref<Reservoir> r, f32 u) noexcept -> bool {
            auto replaced = u < math::guarded_div(r.w_sum, (w_sum + r.w_sum));
            if (replaced) p_hat = r.p_hat;
            w_sum += r.w_sum;
            M += r.M;
            return replaced;
        }

        auto shift(f32 J, f32 mis) noexcept -> void {
            w_sum = p_hat * W * mis * J;
        }

        auto finalize() noexcept -> void {
            W = math::guarded_div(w_sum, p_hat);
        }
    };
}
