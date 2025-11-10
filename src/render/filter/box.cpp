#include <metatron/render/filter/box.hpp>

namespace mtt::filter {
    Box_Filter::Box_Filter(Descriptor const& desc) noexcept: radius(desc.radius) {}

    auto Box_Filter::operator()(math::Vector<f32, 2> const& p) const noexcept -> f32 {
        return math::abs(p) <= radius ? 1.f : 0.f;
    }

    auto Box_Filter::sample(math::Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction> {
        auto p = math::lerp(-radius, radius, u);
        auto w = (*this)(p);
        return filter::Interaction{p, w, 1.f / (4.f * radius[0] * radius[1])};
    }
}
