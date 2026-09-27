#pragma once
#include <metatron/render/photo/lens/interaction.hpp>

namespace mtt::photo {
    struct Pinhole_Lens final {
        f32 focal_distance = 0.035f;
        auto sample(fv2 const& o, fv2 const& u) const noexcept -> lens::Interaction;
    };
}

