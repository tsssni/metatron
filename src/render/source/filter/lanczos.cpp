#include <metatron/render/filter/lanczos.hpp>
#include <metatron/core/math/trigonometric.hpp>

namespace mtt::filter {
    Lanczos_Filter::Lanczos_Filter(cref<Descriptor> desc) noexcept:
    radius(desc.radius), tau(desc.tau) {
        auto matrix = fm<64, 64>{};
        for (auto j = 0uz; j < 64; ++j) {
            auto y = math::lerp(-radius[1], radius[1], (f32(j) + 0.5f) / 64.f);
            for (auto i = 0uz; i < 64; ++i) {
                auto x = math::lerp(-radius[0], radius[0], (f32(i) + 0.5f) / 64.f);
                matrix[j][i] = (*this)({x, y});
            }
        }
        distribution = math::Planar_Distribution{
            std::span{matrix.data(), matrix.size()},
            {64, 64},
            {-radius[1], -radius[0]},
            {radius[1], radius[0]}
        };
    }

    auto Lanczos_Filter::operator()(cref<fv2> p) const noexcept -> f32 {
        auto v = foreach([&](f32 x, usize i) -> f32 {
            return math::abs(x) > radius[i] ? 0.f : math::windowed_sinc(x, tau);
        }, p);
        return prod(v);
    }

    auto Lanczos_Filter::sample(cref<fv2> u) const noexcept -> opt<Interaction> {
        auto p = distribution.sample(u);
        auto w = (*this)(math::reverse(p));
        auto pdf = distribution.pdf(p);
        return Interaction{p, w, pdf};
    }
}
