#include <metatron/render/filter/gaussian.hpp>
#include <metatron/core/math/gaussian.hpp>

namespace mtt::filter {
    Gaussian_Filter::Gaussian_Filter(cref<Descriptor> desc) noexcept:
    radius(desc.radius), sigma(desc.sigma) {
        auto matrix = fm<64, 64>{};
        for (auto j = 0uz; j < 64; ++j) {
            auto y = math::lerp(-radius[1], radius[1], (f32(j) + 0.5f) / 64.f);
            for (auto i = 0uz; i < 64; ++i) {
                auto x = math::lerp(-radius[0], radius[0], (f32(i) + 0.5f) / 64.f);
                matrix[j][i] = math::abs((*this)({x, y}));
            }
        }

        auto& vec = stl::vector<math::Planar_Distribution>::instance();
        distr = vec.emplace_back(
            std::span{matrix.data(), matrix.size()},
            iv2{64, 64},
            fv2{-radius[1], -radius[0]},
            fv2{radius[1], radius[0]}
        );
    }

    auto Gaussian_Filter::operator()(cref<fv2> p) const noexcept -> f32 {
        auto vx = math::gaussian(p[0], 0.f, sigma) - math::gaussian(radius[0], 0.f, sigma);
        auto vy = math::gaussian(p[1], 0.f, sigma) - math::gaussian(radius[1], 0.f, sigma);
        return vx * vy;
    }

    auto Gaussian_Filter::sample(cref<fv2> u) const noexcept -> opt<filter::Interaction> {
        auto p = distr->sample(u);
        auto w = (*this)(math::reverse(p));
        auto pdf = distr->pdf(p);
        return Interaction{p, w, pdf};
    }
}
