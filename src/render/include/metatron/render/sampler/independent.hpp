#pragma once
#include <metatron/render/sampler/sampler.hpp>
#include <random>

namespace mtt::sampler {
    struct Independent_Sampler final {
        auto start(ref<Context> ctx) const noexcept -> void;
        auto generate_1d(ref<Context> ctx) const noexcept -> f32;
        auto generate_2d(ref<Context> ctx) const noexcept -> fv2;
        auto generate_pixel_2d(ref<Context> ctx) const noexcept -> fv2;
    };
}
