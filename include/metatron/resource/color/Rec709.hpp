#pragma once
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::color {
    struct Rec709_Transfer_Function final {
        struct Descriptor final {};
        Rec709_Transfer_Function(cref<Descriptor>) noexcept {}
        Rec709_Transfer_Function() noexcept = default;

        auto transfer(f32 x) const noexcept -> f32 {
            if (x <= 0.0031308f) return 12.92f * x;
            else return 1.055f * math::pow(x, 1.f / 2.4f) - 0.055f;
        }

        auto linearize(f32 x) const noexcept -> f32 {
            if (x <= 0.04045f) return x / 12.92f;
            else return math::pow((x + 0.055f) / 1.055f, 2.4f);
        }
    private:
        u32 padding;
    };
}
