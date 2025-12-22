#include <metatron/render/filter/box.hpp>

namespace mtt::filter {
    Box_Filter::Box_Filter(cref<Descriptor> desc) noexcept: radius(desc.radius) {}

    auto Box_Filter::operator()(cref<fv2> p) const noexcept -> f32 {
        return math::abs(p) <= radius ? 1.f : 0.f;
    }

    auto Box_Filter::sample(cref<fv2> u) const noexcept -> opt<filter::Interaction> {
        auto p = math::lerp(-radius, radius, u);
        auto w = (*this)(p);
        return Interaction{p, w, 1.f / (4.f * radius[0] * radius[1])};
    }
}
