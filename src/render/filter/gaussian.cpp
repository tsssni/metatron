#include <metatron/render/filter/gaussian.hpp>
#include <metatron/core/math/gaussian.hpp>

namespace mtt::filter {
    Gaussian_Filter::Gaussian_Filter(
        math::Vector<f32, 2> const& radius,
        f32 sigma
    ) noexcept: radius(radius) {
        auto matrix = math::Matrix<f32, 64, 64>{};
        for (auto j = 0uz; j < 64; ++j) {
            auto y = math::lerp(-radius[1], radius[1], (f32(j) + 0.5f) / 64.f);
            for (auto i = 0uz; i < 64; ++i) {
                auto x = math::lerp(-radius[0], radius[0], (f32(i) + 0.5f) / 64.f);
                matrix[j][i] = math::abs((*this)({x, y}));
            }
        }
        piecewise = math::Piecewise_Distribution<2>{
            std::span{matrix.data(), matrix.size()},
            {64, 64},
            {-radius[1], -radius[0]},
            {radius[1], radius[0]}
        };
    }

    auto Gaussian_Filter::operator()(math::Vector<f32, 2> const& p) const noexcept -> f32 {
        auto vx = math::gaussian(p[0], 0.f, sigma) - math::gaussian(radius[0], 0.f, sigma);
        auto vy = math::gaussian(p[1], 0.f, sigma) - math::gaussian(radius[1], 0.f, sigma);
        return vx * vy;
    }

    auto Gaussian_Filter::sample(math::Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction> {
        auto p = piecewise.sample(u);
        auto w = (*this)(math::reverse(p));
        auto pdf = piecewise.pdf(p);
        return Interaction{p, w, pdf};
    }
}
