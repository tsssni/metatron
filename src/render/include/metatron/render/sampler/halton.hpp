#pragma once
#include <metatron/render/sampler/sampler.hpp>

namespace mtt::sampler {
    // halton with owen scrambling: https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
    struct Halton_Sampler final {
        struct Descriptor final { uv2 scale_exponential = {7, 4}; };
        Halton_Sampler(cref<Descriptor> desc) noexcept;
        Halton_Sampler() noexcept = default;

        auto start(ref<Context> ctx) const noexcept -> void;
        auto generate_1d(ref<Context> ctx) const noexcept -> f32;
        auto generate_2d(ref<Context> ctx) const noexcept -> fv2;
        auto generate_pixel_2d(ref<Context> ctx) const noexcept -> fv2;

    private:
        uv2 exponential;
        uv2 scale;
        uv2 scale_mulinv;
        u32 stride;
    };
}
