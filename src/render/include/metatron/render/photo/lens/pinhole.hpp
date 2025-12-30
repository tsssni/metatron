#pragma once
#include <metatron/render/photo/lens/lens.hpp>

namespace mtt::photo {
    struct Pinhole_Lens final {
        f32 focal_distance = 0.035f;
        auto sample(cref<fv2> o, cref<fv2> u) const noexcept -> opt<lens::Interaction>;
    };
}

