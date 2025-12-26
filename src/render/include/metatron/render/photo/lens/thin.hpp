#pragma once
#include <metatron/render/photo/lens/lens.hpp>

namespace mtt::photo {
    struct Thin_Lens final {
        struct Descriptor final {
            f32 aperture = 5.6f;
            f32 focal_length = 0.035f;
            f32 focus_distance = 10.f;
        };
        Thin_Lens(cref<Descriptor> desc) noexcept;
        auto sample(cref<fv2> o, cref<fv2> u) const noexcept -> opt<lens::Interaction>;

    private:
        f32 aperture;
        f32 focal_length;
        f32 focus_distance;
        f32 focal_distance;
    };
}

