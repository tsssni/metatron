#pragma once
#include <metatron/render/sampler/independent.hpp>
#include <metatron/render/sampler/halton.hpp>
#include <metatron/render/sampler/z-sobol.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::sampler {
    auto init() noexcept -> void;

    struct Sampler final: stl::polymorph<Sampler
    , Independent_Sampler
    , Halton_Sampler
    , Z_Sobol_Sampler> {
        using polymorph::polymorph;

        auto start(Context& ctx) const noexcept -> void {
            visit([&](auto* p) noexcept { p->start(ctx); });
        }
        auto generate_1d(Context& ctx) const noexcept -> f32 {
            return visit([&](auto* p) noexcept { return p->generate_1d(ctx); });
        }
        auto generate_2d(Context& ctx) const noexcept -> fv2 {
            return visit([&](auto* p) noexcept { return p->generate_2d(ctx); });
        }
        auto generate_pixel_2d(Context& ctx) const noexcept -> fv2 {
            return visit([&](auto* p) noexcept { return p->generate_pixel_2d(ctx); });
        }
    };
}

namespace mtt::sampler::proxy {
    struct Sampler final {
        sampler::Sampler sampler;
        Context ctx;
        auto start() noexcept -> void { sampler.start(ctx); }
        auto generate_1d() noexcept -> f32 { return sampler.generate_1d(ctx); }
        auto generate_2d() noexcept -> fv2 { return sampler.generate_2d(ctx); }
        auto generate_pixel_2d() noexcept -> fv2 { return sampler.generate_pixel_2d(ctx); }
    };
}
