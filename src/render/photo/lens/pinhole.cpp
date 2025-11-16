#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/core/math/constant.hpp>

namespace mtt::photo {
    auto Pinhole_Lens::sample(fv2 o, fv2 u) const noexcept -> opt<lens::Interaction> {
        auto p = fv3{0.f, 0.f, focal_distance};
        return lens::Interaction{{{0.f}, math::normalize(fv3{-o, focal_distance})}, 1.f};
    }
}
