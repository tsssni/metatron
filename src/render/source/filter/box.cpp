#include <metatron/render/filter/box.hpp>

namespace mtt::filter {
    Box_Filter::Box_Filter(Descriptor const& desc) noexcept: radius(desc.radius) {}

    auto Box_Filter::operator()(fv2 const& p) const noexcept -> f32 {
        return math::abs(p) <= radius ? 1.f : 0.f;
    }

    auto Box_Filter::sample(fv2 const& u) const noexcept -> filter::Interaction {
        auto p = math::lerp(-radius, radius, u);
        auto w = (*this)(p);
        return {p, w, 1.f / (4.f * radius[0] * radius[1])};
    }
}
