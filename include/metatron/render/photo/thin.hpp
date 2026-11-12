#pragma once
#include <metatron/render/photo/interaction.hpp>

namespace mtt::render {

struct [[mtt::reflection]] Thin_Lens final {
    f32 aperture;
    f32 focal_length;
    f32 focus_distance;
    f32 focal_distance;
    Thin_Lens(f32 aperture, f32 focal_length, f32 focus_distance) noexcept;
    auto sample(fv2 const& o, fv2 const& u) const noexcept -> Lens_Interaction;
};

}
