#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::sampler {
    struct Context final {
        uv4 data;
        uv2 pixel;
        uv2 size;
        u32 idx;
        u32 spp;
        u32 dim;
        u32 seed;
    };

    MTT_POLY_METHOD(sampler_start, start);
    MTT_POLY_METHOD(sampler_generate_1d, generate_1d);
    MTT_POLY_METHOD(sampler_generate_2d, generate_2d);
    MTT_POLY_METHOD(sampler_generate_pixel_2d, generate_pixel_2d);

    struct Sampler final: pro::facade_builder
    ::add_convention<sampler_start, auto (ref<Context> ctx) const noexcept -> void>
    ::add_convention<sampler_generate_1d, auto (ref<Context> ctx) const noexcept -> f32>
    ::add_convention<sampler_generate_2d, auto (ref<Context> ctx) const noexcept -> fv2>
    ::add_convention<sampler_generate_pixel_2d, auto (ref<Context> ctx) const noexcept -> fv2>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
