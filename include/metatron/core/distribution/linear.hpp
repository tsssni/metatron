#pragma once

namespace mtt::core {

struct Linear_Distribution final {
    f32 a;
    f32 b;
    auto sample(f32 u) const noexcept -> f32;
    auto pdf(f32 x) const noexcept -> f32;
};

auto inline Linear_Distribution::sample(f32 u) const noexcept -> f32 {
    auto a2 = a * a;
    auto b2 = b * b;
    auto x = u * (a + b) / (a + pow<1, 2>(lerp(a2, b2, u)));
    return x;
}

auto inline Linear_Distribution::pdf(f32 x) const noexcept -> f32 {
    return 2.f * lerp(a, b, x) / (a + b);
}

struct Bilinear_Distribution final {
    f32 a;
    f32 b;
    f32 c;
    f32 d;

    auto sample(fv2 const& u) const noexcept -> fv2;

    auto pdf(fv2 const& x) const noexcept -> f32;
};

auto inline Bilinear_Distribution::sample(fv2 const& u) const noexcept -> fv2 {
    auto x = Linear_Distribution{a + b, c + d}.sample(u[0]);
    auto y = Linear_Distribution{lerp(a, c, x), lerp(b, d, x)}.sample(u[1]);
    return {x, y};
}

auto inline Bilinear_Distribution::pdf(fv2 const& x) const noexcept -> f32 {
    return 1.f
        * Linear_Distribution{a + b, c + d}.pdf(x[0])
        * Linear_Distribution{lerp(a, c, x[0]), lerp(b, d, x[0])}.pdf(x[1]);
}

}
