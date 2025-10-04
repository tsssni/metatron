#pragma once
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::math {
    struct Linear_Distribution final {
        Linear_Distribution(f32 a, f32 b) noexcept
        : a(a), b(b) {}

        auto sample(f32 u) const noexcept -> f32 {
            auto a2 = a * a;
            auto b2 = b * b;
            auto x =  u * (a + b) / (a + math::sqrt(math::lerp(a2, b2, u)));
            return x;
        }

        auto pdf(f32 x) const noexcept -> f32 {
            return 2.f * math::lerp(a, b, x) / (a + b);
        }

    private:
        f32 a;
        f32 b;
    };

    struct Bilinear_Distribution final {
        Bilinear_Distribution(f32 a, f32 b, f32 c, f32 d) noexcept
        : a(a), b(b), c(c), d(d) {}

        auto sample(math::Vector<f32, 2> u) const noexcept -> math::Vector<f32, 2> {
            auto x = Linear_Distribution{a + b, c + d}.sample(u[0]);
            auto y = Linear_Distribution{math::lerp(a, c, x), math::lerp(b, d, x)}.sample(u[1]);
            return {x, y};
        }

        auto pdf(math::Vector<f32, 2> x) const noexcept -> f32 {
            return 1.f
            * Linear_Distribution{a + b, c + d}.pdf(x[0])
            * Linear_Distribution{math::lerp(a, c, x[0]), math::lerp(b, d, x[0])}.pdf(x[1]);
        }

    private:
        f32 a;
        f32 b;
        f32 c;
        f32 d;
    };
}
