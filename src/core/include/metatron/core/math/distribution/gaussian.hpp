#pragma once
#include <metatron/core/math/gaussian.hpp>

namespace mtt::math {
    struct Gaussian_Distribution final {
        Gaussian_Distribution(f32 mu, f32 sigma) noexcept: mu(mu), sigma(sigma) {}

        auto pdf(f32 x) const noexcept -> f32 {
            return gaussian(x, mu, sigma);
        }

    private:
        f32 mu;
        f32 sigma;
    };

    struct Truncated_Gaussian_Distribution final {
        Truncated_Gaussian_Distribution() noexcept {}
        Truncated_Gaussian_Distribution(f32 mu, f32 sigma, f32 a, f32 b) noexcept:
        mu(mu), sigma(sigma),
        cdf_a(gaussian_cdf(a, mu, sigma)), cdf_b(gaussian_cdf(b, mu, sigma)) {}

        auto sample(f32 u) const noexcept -> f32 {
            auto s = std::lerp(cdf_a, cdf_b, u);
            s = std::clamp(s, math::epsilon<f32>, 1.f - math::epsilon<f32>);
            return std::sqrt(2.f) * erfinv(2.f * s - 1.f) * sigma + mu;
        }

        auto pdf(f32 x) const noexcept -> f32 {
            return math::guarded_div(gaussian(x, mu, sigma), (cdf_b - cdf_a));
        }

    private:
        f32 mu;
        f32 sigma;
        f32 cdf_a;
        f32 cdf_b;
    };
}
