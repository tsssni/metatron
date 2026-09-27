#pragma once
#include <metatron/render/sampler/context.hpp>
#include <random>

namespace mtt::sampler {
    struct Independent_Sampler final {
        auto start(Context& ctx) const noexcept -> void;
        auto generate_1d(Context& ctx) const noexcept -> f32;
        auto generate_2d(Context& ctx) const noexcept -> fv2;
        auto generate_pixel_2d(Context& ctx) const noexcept -> fv2;
    };
}
