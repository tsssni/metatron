#pragma once
#include <metatron/render/sampler/sampler.hpp>

namespace mtt::sampler {
    // halton with owen scrambling: https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
    struct Halton_Sampler final {
        struct Descriptor final {
            iv2 scale_exponential = {7, 4};
        };
        Halton_Sampler() noexcept = default;
        Halton_Sampler(cref<Halton_Sampler>) noexcept = default;
        Halton_Sampler(cref<Descriptor> desc) noexcept;
        auto start(Context ctx) noexcept -> void;
        auto generate_1d() noexcept -> f32;
        auto generate_2d() noexcept -> fv2;
        auto generate_pixel_2d() noexcept -> fv2;
    
    private:
        iv2 pixel;
        iv2 exponential;
        iv2 scale;
        iv2 scale_mulinv;
        i32 idx;
        i32 stride;
        usize seed;
        usize dim;
        usize halton_idx;
    };
}
