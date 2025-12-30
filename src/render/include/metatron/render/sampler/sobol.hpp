#pragma once
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::sampler {
    auto constexpr num_sobol_dimensions = 2;
    auto constexpr sobol_matrix_size = 52;

    struct Sobol_Sampler final {
        struct Descriptor final {};
        Sobol_Sampler(cref<Descriptor>) noexcept;
        Sobol_Sampler() noexcept = default;

        auto static init() noexcept -> void;
        auto start(ref<Context> ctx) const noexcept -> void;
        auto generate_1d(ref<Context> ctx) const noexcept -> f32;
        auto generate_2d(ref<Context> ctx) const noexcept -> fv2;
        auto generate_pixel_2d(ref<Context> ctx) const noexcept -> fv2;

    private:
        auto permute_idx(cref<Context> ctx) const noexcept -> u64;
        auto sobol(u64 idx, i32 dim, u32 hash) const noexcept -> f32;

        buf<u32> static sobol_matrices;
        buf<u32> matrices = {};
    };
}
