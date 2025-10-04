#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::math {
    MTT_POLY_METHOD(sampler_start, start);
    MTT_POLY_METHOD(sampler_generate_1d, generate_1d);
    MTT_POLY_METHOD(sampler_generate_2d, generate_2d);
    MTT_POLY_METHOD(sampler_generate_pixel_2d, generate_pixel_2d);

    struct Sampler final: pro::facade_builder
    ::add_convention<sampler_start, auto (
        math::Vector<usize, 2> const& pixel, usize idx, usize dim
    ) noexcept -> void>
    ::add_convention<sampler_generate_1d, auto () noexcept -> f32>
    ::add_convention<sampler_generate_2d, auto () noexcept -> math::Vector<f32, 2>>
    ::add_convention<sampler_generate_pixel_2d, auto () noexcept -> math::Vector<f32, 2>>
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
