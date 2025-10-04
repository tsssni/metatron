#pragma once
#include <metatron/core/math/filter/filter.hpp>

namespace mtt::math {
    struct Box_Filter final {
        Box_Filter(Vector<f32, 2> const& radius = {0.5f}) noexcept: radius(radius) {}

        auto operator()(Vector<f32, 2> const& p) const noexcept -> f32 {
            return math::abs(p) <= radius ? 1.f : 0.f;
        }

        auto sample(Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction> {
            auto p = math::lerp(-radius, radius, u);
            auto w = (*this)(p);
            return filter::Interaction{p, w, 1.f / (4.f * radius[0] * radius[1])};
        }

    private:
        Vector<f32, 2> radius;
    };
}
