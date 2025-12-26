#pragma once
#include <metatron/render/sampler/sampler.hpp>

namespace mtt::sampler {
    // halton with owen scrambling: https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
    struct Halton_Sampler final {
        Halton_Sampler() noexcept = default;
        Halton_Sampler(rref<Halton_Sampler>) noexcept = default;
        Halton_Sampler(cref<Halton_Sampler>) noexcept = default;
        auto operator=(rref<Halton_Sampler>) noexcept -> Halton_Sampler& = default;
        auto operator=(cref<Halton_Sampler>) noexcept -> Halton_Sampler& = default;

        struct Descriptor final {
            uv2 scale_exponential = {7, 4};
        };
        Halton_Sampler(cref<Descriptor> desc) noexcept;
        auto start(Context ctx) noexcept -> void;
        auto generate_1d() noexcept -> f32;
        auto generate_2d() noexcept -> fv2;
        auto generate_pixel_2d() noexcept -> fv2;
    
    private:
        uv2 pixel;
        uv2 exponential;
        uv2 scale;
        uv2 scale_mulinv;
        u32 idx;
        u32 stride;
        u32 seed;
        u32 dim;
        u32 halton_idx;
    };
}
