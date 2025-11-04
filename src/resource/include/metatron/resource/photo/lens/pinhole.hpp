#pragma once
#include <metatron/resource/photo/lens/lens.hpp>

namespace mtt::photo {
    struct Pinhole_Lens final {
        f32 focal_distance;
        auto sample(math::Vector<f32, 2> o, math::Vector<f32, 2> u) const noexcept -> std::optional<lens::Interaction>;
    };
}

