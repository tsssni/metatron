#pragma once
#include <metatron/core/math/vector.hpp>
#include <random>

namespace mtt::sampler {
    struct Independent_Sampler final {
        Independent_Sampler(i32 seed) noexcept;
        Independent_Sampler(Independent_Sampler const&) noexcept;

        auto start(math::Vector<usize, 2> const& pixel, usize idx, usize dim = 0uz) noexcept -> void;
        auto generate_1d() noexcept -> f32;
        auto generate_2d() noexcept -> math::Vector<f32, 2>;
        auto generate_pixel_2d() noexcept -> math::Vector<f32, 2>;
    
    private:
        std::mt19937 rng;
        std::uniform_real_distribution<f32> dist;
        math::Vector<i32, 2> pixel;
        i32 idx;
        i32 dim;
    };
}
