#pragma once
#include <metatron/core/math/vector.hpp>
#include <vector>

namespace mtt::sampler {
    auto constexpr num_sobol_dimensions = 2;
    auto constexpr sobol_matrix_size = 52;

    struct Sobol_Sampler final {
        Sobol_Sampler(usize seed, usize spp, math::Vector<usize, 2> size) noexcept;
        Sobol_Sampler(Sobol_Sampler const&) noexcept = default;
        auto static init() noexcept -> void;
        auto start(math::Vector<usize, 2> const& pixel, usize idx, usize dim = 0uz) noexcept -> void;
        auto generate_1d() noexcept -> f32;
        auto generate_2d() noexcept -> math::Vector<f32, 2>;
        auto generate_pixel_2d() noexcept -> math::Vector<f32, 2>;
    
    private:
        auto permute_idx() noexcept -> usize;
        auto sobol(usize idx, i32 dim, u32 hash) noexcept -> f32;

        std::vector<u32> static sobol_matrices;

        i32 log2_spp;
        i32 base4_digits;
        i32 dim;
        usize seed;
        usize morton_idx;
    };
}
