#pragma once
#include <metatron/render/sampler/context.hpp>

namespace mtt::render {

struct [[mtt::reflection]] Z_Sobol_Sampler final {
    buf<u32> matrices;
    Z_Sobol_Sampler() noexcept;

    auto start(Sampler_Context& ctx) const noexcept -> void;
    auto generate_1d(Sampler_Context& ctx) const noexcept -> f32;
    auto generate_2d(Sampler_Context& ctx) const noexcept -> fv2;
    auto generate_pixel_2d(Sampler_Context& ctx) const noexcept -> fv2;

private:
    auto permute_idx(Sampler_Context const& ctx) const noexcept -> u64;
    auto sobol(u64 idx, i32 dim, u32 hash) const noexcept -> f32;
};

}
