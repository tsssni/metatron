#pragma once
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::sampler {
    struct Proxy_Sampler final {
        tag<sampler::Sampler> sampler;
        sampler::Context ctx;

        auto start() noexcept -> void;
        auto generate_1d() noexcept -> f32;
        auto generate_2d() noexcept -> fv2;
        auto generate_pixel_2d() noexcept -> fv2;
    };
}
