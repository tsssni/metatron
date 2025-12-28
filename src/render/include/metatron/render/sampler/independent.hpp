#pragma once
#include <metatron/render/sampler/sampler.hpp>
#include <random>

namespace mtt::sampler {
    struct Independent_Sampler final {
        struct Descriptor final {};
        Independent_Sampler(cref<Descriptor>) noexcept;
        Independent_Sampler() noexcept = default;
        Independent_Sampler(rref<Independent_Sampler>) noexcept = default;
        Independent_Sampler(cref<Independent_Sampler>) noexcept = default;
        auto operator=(rref<Independent_Sampler>) noexcept -> Independent_Sampler& = default;
        auto operator=(cref<Independent_Sampler>) noexcept -> Independent_Sampler& = default;

        auto start(Context ctx) noexcept -> void;
        auto generate_1d() noexcept -> f32;
        auto generate_2d() noexcept -> fv2;
        auto generate_pixel_2d() noexcept -> fv2;
    
    private:
        std::minstd_rand rng;
        std::uniform_real_distribution<f32> distr;
        iv2 pixel;
        i32 idx;
        i32 dim;
    };
}
