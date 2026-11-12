#pragma once
#include <metatron/render/sampler/context.hpp>

namespace mtt::render {

// halton with owen scrambling: https://pbr-book.org/4ed/Sampling_and_Reconstruction/Halton_Sampler
struct [[mtt::reflection]] Halton_Sampler final {
    uv2 exponential;
    uv2 scale;
    uv2 scale_mulinv;
    u32 stride;
    Halton_Sampler(uv2 scale_exponential = {7, 4}) noexcept;

    auto start(Sampler_Context& ctx) const noexcept -> void;
    auto generate_1d(Sampler_Context& ctx) const noexcept -> f32;
    auto generate_2d(Sampler_Context& ctx) const noexcept -> fv2;
    auto generate_pixel_2d(Sampler_Context& ctx) const noexcept -> fv2;
};

}
