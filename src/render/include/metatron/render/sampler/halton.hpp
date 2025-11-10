#pragma once
#include <metatron/render/sampler/sampler.hpp>

namespace mtt::sampler {
    // halton with owen scrambling: https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
    struct Halton_Sampler final {
        struct Descriptor final {
            math::Vector<i32, 2> scale_exponential = {7, 4};
        };
        Halton_Sampler() noexcept = default;
        Halton_Sampler(Halton_Sampler const&) noexcept = default;
        Halton_Sampler(Descriptor const& desc) noexcept;
        auto start(Context ctx) noexcept -> void;
        auto generate_1d() noexcept -> f32;
        auto generate_2d() noexcept -> math::Vector<f32, 2>;
        auto generate_pixel_2d() noexcept -> math::Vector<f32, 2>;
    
    private:
        math::Vector<i32, 2> pixel;
        math::Vector<i32, 2> exponential;
        math::Vector<i32, 2> scale;
        math::Vector<i32, 2> scale_mulinv;
        i32 idx;
        i32 stride;
        usize seed;
        usize dim;
        usize halton_idx;
    };
}
