#pragma once
#include <metatron/render/sampler/halton.hpp>
#include <metatron/render/sampler/independent.hpp>
#include <metatron/render/sampler/z-sobol.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::render {

struct Sampler final : core::protocol<Independent_Sampler, Halton_Sampler, Z_Sobol_Sampler> {
    using protocol::protocol;

    auto start(Sampler_Context& ctx) const noexcept -> void;
    auto generate_1d(Sampler_Context& ctx) const noexcept -> f32;
    auto generate_2d(Sampler_Context& ctx) const noexcept -> fv2;
    auto generate_pixel_2d(Sampler_Context& ctx) const noexcept -> fv2;
};

auto inline Sampler::start(Sampler_Context& ctx) const noexcept -> void {
    visit([&](auto* p) noexcept { p->start(ctx); });
}

auto inline Sampler::generate_1d(Sampler_Context& ctx) const noexcept -> f32 {
    return visit([&](auto* p) noexcept { return p->generate_1d(ctx); });
}

auto inline Sampler::generate_2d(Sampler_Context& ctx) const noexcept -> fv2 {
    return visit([&](auto* p) noexcept { return p->generate_2d(ctx); });
}

auto inline Sampler::generate_pixel_2d(Sampler_Context& ctx) const noexcept -> fv2 {
    return visit([&](auto* p) noexcept { return p->generate_pixel_2d(ctx); });
}

struct Generator final {
    Sampler sampler;
    Sampler_Context ctx;
    auto start() noexcept -> void;
    auto generate_1d() noexcept -> f32;
    auto generate_2d() noexcept -> fv2;
    auto generate_pixel_2d() noexcept -> fv2;
};

auto inline Generator::start() noexcept -> void { sampler.start(ctx); }
auto inline Generator::generate_1d() noexcept -> f32 { return sampler.generate_1d(ctx); }
auto inline Generator::generate_2d() noexcept -> fv2 { return sampler.generate_2d(ctx); }
auto inline Generator::generate_pixel_2d() noexcept -> fv2 { return sampler.generate_pixel_2d(ctx); }

}
