#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>
#include <metatron/resource/serde/args.hpp>

namespace mtt::renderer {
    struct Renderer final: stl::capsule<Renderer> {
        struct Impl;
        struct Descriptor final {
            monte_carlo::Integrator integrator = monte_carlo::Integrator::entity("/renderer/default/integrator");
            accel::Acceleration accel = accel::Acceleration::entity("/renderer/default/accel");
            emitter::Emitter emitter = emitter::Emitter::entity("/renderer/default/emitter");
            sampler::Sampler sampler = sampler::Sampler::entity("/renderer/default/sampler");
            filter::Filter filter = filter::Filter::entity("/renderer/default/filter");
            photo::Lens lens = photo::Lens::entity("/renderer/default/lens");
            photo::proxy::Film film = {};
        };
        Renderer() noexcept = default;
        Renderer(rref<Descriptor> desc) noexcept;
        auto render(cref<scene::Args> args) noexcept -> void;
    };
}
