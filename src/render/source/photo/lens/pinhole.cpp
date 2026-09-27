#include <metatron/render/photo/lens/pinhole.hpp>

namespace mtt::photo {
    auto Pinhole_Lens::sample(fv2 const& o, fv2 const& u) const noexcept -> lens::Interaction {
        auto p = fv3{0.f, 0.f, focal_distance};
        return {{{0.f}, math::normalize(fv3{-o, focal_distance})}, 1.f};
    }
}
