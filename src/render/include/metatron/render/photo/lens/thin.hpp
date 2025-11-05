#pragma once
#include <metatron/render/photo/lens/lens.hpp>

namespace mtt::photo {
    struct Thin_Lens final {
        struct Descriptor final {
            f32 aperture;
            f32 focal_length;
            f32 focus_distance;
        };
        Thin_Lens(Descriptor const& desc) noexcept;
        auto sample(math::Vector<f32, 2> o, math::Vector<f32, 2> u) const noexcept -> std::optional<lens::Interaction>;
    
    private:
        f32 aperture;
        f32 focal_length;
        f32 focus_distance;
        f32 focal_distance;
    };
}

