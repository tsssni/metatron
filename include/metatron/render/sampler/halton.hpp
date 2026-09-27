#pragma once
#include <metatron/render/sampler/context.hpp>

namespace mtt::sampler {
    // halton with owen scrambling: https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
    struct Halton_Sampler final {
        struct Descriptor final { uv2 scale_exponential = {7, 4}; };
        Halton_Sampler(Descriptor const& desc) noexcept;
        Halton_Sampler() noexcept = default;

        auto start(Context& ctx) const noexcept -> void;
        auto generate_1d(Context& ctx) const noexcept -> f32;
        auto generate_2d(Context& ctx) const noexcept -> fv2;
        auto generate_pixel_2d(Context& ctx) const noexcept -> fv2;

    private:
        uv2 exponential;
        uv2 scale;
        uv2 scale_mulinv;
        u32 stride;
    };
}
