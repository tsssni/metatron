#pragma once
#include <metatron/render/filter/filter.hpp>
#include <metatron/core/math/distribution/piecewise.hpp>

namespace mtt::filter {
    struct Gaussian_Filter final {
        Gaussian_Filter(
            math::Vector<f32, 2> const& radius = {1.5f},
            f32 sigma = 0.5f
        ) noexcept;
        auto operator()(math::Vector<f32, 2> const& p) const noexcept -> f32;
        auto sample(math::Vector<f32, 2> const& u) const noexcept -> std::optional<Interaction>;

    private:
        math::Piecewise_Distribution<2> piecewise;
        math::Vector<f32, 2> radius;
        f32 sigma;
    };
}
