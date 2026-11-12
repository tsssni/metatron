#include <metatron/render/photo/pinhole.hpp>

namespace mtt::render {

auto Pinhole_Lens::sample(fv2 const& o, fv2 const& u) const noexcept -> Lens_Interaction {
    auto p = fv3{0.f, 0.f, focal_distance};
    return {{{0.f}, core::normalize(fv3{-o, focal_distance})}, 1.f};
}

}
