#pragma once
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::sampler {
    auto constexpr num_sobol_dimensions = 2;
    auto constexpr sobol_matrix_size = 52;

    struct Sobol_Sampler final {
        Sobol_Sampler() noexcept;
        Sobol_Sampler(rref<Sobol_Sampler>) noexcept = default;
        Sobol_Sampler(cref<Sobol_Sampler>) noexcept = default;
        auto operator=(rref<Sobol_Sampler>) noexcept -> Sobol_Sampler& = default;
        auto operator=(cref<Sobol_Sampler>) noexcept -> Sobol_Sampler& = default;

        auto static init() noexcept -> void;
        auto start(Context ctx) noexcept -> void;
        auto generate_1d() noexcept -> f32;
        auto generate_2d() noexcept -> fv2;
        auto generate_pixel_2d() noexcept -> fv2;

    // private:
        auto permute_idx() noexcept -> u64;
        auto sobol(u64 idx, i32 dim, u32 hash) noexcept -> f32;

        buf<u32> static sobol_matrices;

        i32 log2_spp;
        i32 base4_digits;
        i32 dim;
        u32 seed;
        u32 morton_idx;
        buf<u32> matrices = {};
    };
}
