#pragma once
#include <metatron/core/math/gaussian.hpp>

namespace mtt::core {

struct Gaussian_Distribution final {
    f32 mu;
    f32 sigma;
    auto pdf(f32 x) const noexcept -> f32;
};

auto inline Gaussian_Distribution::pdf(f32 x) const noexcept -> f32 {
    return gaussian(x, mu, sigma);
}

struct Truncated_Gaussian_Distribution final {
    f32 mu;
    f32 sigma;
    f32 cdf_a;
    f32 cdf_b;
    auto update(f32 mu, f32 sigma, f32 a, f32 b) noexcept -> void;
    auto sample(f32 u) const noexcept -> f32;
    auto pdf(f32 x) const noexcept -> f32;
};

auto inline Truncated_Gaussian_Distribution::update(f32 mu, f32 sigma, f32 a, f32 b) noexcept -> void {
    this->mu = mu;
    this->sigma = sigma;
    cdf_a = gaussian_cdf(a, mu, sigma);
    cdf_b = gaussian_cdf(b, mu, sigma);
}

auto inline Truncated_Gaussian_Distribution::sample(f32 u) const noexcept -> f32 {
    auto s = lerp(cdf_a, cdf_b, u);
    s = clamp(s, epsilon<f32>, 1.f - epsilon<f32>);
    return pow<1, 2>(2.f) * erfinv(2.f * s - 1.f) * sigma + mu;
}

auto inline Truncated_Gaussian_Distribution::pdf(f32 x) const noexcept -> f32 {
    return guarded_div(gaussian(x, mu, sigma), (cdf_b - cdf_a));
}

}
