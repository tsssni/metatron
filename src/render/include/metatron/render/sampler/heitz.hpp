#pragma once
#include <metatron/render/sampler/context.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::sampler {
    auto constexpr heitz_sobol_n = 4096uz;
    auto constexpr heitz_sobol_d = 256uz;
    auto constexpr heitz_mask_size = 128uz;
    auto constexpr heitz_pair_d = 2uz;

    struct Heitz_Sampler final {
        struct Descriptor final {};
        Heitz_Sampler(cref<Descriptor>) noexcept;
        Heitz_Sampler() noexcept = default;

        auto static init() noexcept -> void;

        auto start(ref<Context> ctx) const noexcept -> void;
        auto generate_1d(ref<Context> ctx) const noexcept -> f32;
        auto generate_2d(ref<Context> ctx) const noexcept -> fv2;
        auto generate_pixel_2d(ref<Context> ctx) const noexcept -> fv2;

    private:
        buf<u32> static heitz_sobol;
        buf<u32> static heitz_mask;
        buf<u32> sobol = {};
        buf<u32> mask = {};
    };
}
