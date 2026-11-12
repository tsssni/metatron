#pragma once
#include <metatron/render/sampler/context.hpp>
#include <random>

namespace mtt::render {

struct [[mtt::reflection]] Independent_Sampler final {
    auto start(Sampler_Context& ctx) const noexcept -> void;
    auto generate_1d(Sampler_Context& ctx) const noexcept -> f32;
    auto generate_2d(Sampler_Context& ctx) const noexcept -> fv2;
    auto generate_pixel_2d(Sampler_Context& ctx) const noexcept -> fv2;
};

}
