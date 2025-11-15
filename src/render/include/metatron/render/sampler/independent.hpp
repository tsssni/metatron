#pragma once
#include <metatron/render/sampler/sampler.hpp>
#include <random>

namespace mtt::sampler {
    struct Independent_Sampler final {
        Independent_Sampler() noexcept;
        Independent_Sampler(cref<Independent_Sampler>) noexcept = default;

        auto start(Context ctx) noexcept -> void;
        auto generate_1d() noexcept -> f32;
        auto generate_2d() noexcept -> fv2;
        auto generate_pixel_2d() noexcept -> fv2;
    
    private:
        std::mt19937 rng;
        std::uniform_real_distribution<f32> distr;
        iv2 pixel;
        i32 idx;
        i32 dim;
    };
}
