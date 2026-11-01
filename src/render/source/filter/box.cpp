#include <metatron/render/filter/box.hpp>

namespace mtt::render {

auto Box_Filter::operator()(fv2 const& p) const noexcept -> f32 {
    return core::abs(p) <= radius ? 1.f : 0.f;
}

auto Box_Filter::sample(fv2 const& u) const noexcept -> render::Filter_Interaction {
    auto p = core::lerp(-radius, radius, u);
    auto w = (*this)(p);
    return {p, w, 1.f / (4.f * radius[0] * radius[1])};
}

}
