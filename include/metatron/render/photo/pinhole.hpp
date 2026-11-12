#pragma once
#include <metatron/render/photo/interaction.hpp>

namespace mtt::render {

struct [[mtt::reflection]] Pinhole_Lens final {
    f32 focal_distance = 0.035f;
    auto sample(fv2 const& o, fv2 const& u) const noexcept -> Lens_Interaction;
};

}
