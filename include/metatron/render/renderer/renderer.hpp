#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/render/filter/filter.hpp>
#include <metatron/render/photo/lens/lens.hpp>
#include <metatron/render/photo/film.hpp>
#include <metatron/resource/serde/args.hpp>
#include <metatron/core/stl/capsule.hpp>

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
