#pragma once
#include <metatron/render/sampler/context.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::sampler::z_sobol {
    auto init() noexcept -> void;
}

namespace mtt::sampler {
    auto constexpr num_sobol_dimensions = 2u;
    auto constexpr sobol_matrix_size = 52u;

    struct Z_Sobol_Sampler final {
        struct Descriptor final {};
        Z_Sobol_Sampler(Descriptor const&) noexcept;
        Z_Sobol_Sampler() noexcept = default;

        auto start(Context& ctx) const noexcept -> void;
        auto generate_1d(Context& ctx) const noexcept -> f32;
        auto generate_2d(Context& ctx) const noexcept -> fv2;
        auto generate_pixel_2d(Context& ctx) const noexcept -> fv2;

    private:
        friend auto z_sobol::init() noexcept -> void;
        auto permute_idx(Context const& ctx) const noexcept -> u64;
        auto sobol(u64 idx, i32 dim, u32 hash) const noexcept -> f32;

        buf<u32> static sobol_matrices;
        buf<u32> matrices = {};
    };
}
