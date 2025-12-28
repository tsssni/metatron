#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/render/filter/filter.hpp>
#include <metatron/render/photo/lens/lens.hpp>
#include <metatron/render/photo/film.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::renderer {
    struct Renderer final: stl::capsule<Renderer> {
        struct Impl;
        struct Descriptor final {
            tag<monte_carlo::Integrator> integrator = entity<monte_carlo::Integrator>("/renderer/default/integrator");
            tag<accel::Acceleration> accel = entity<accel::Acceleration>("/renderer/default/accel");
            tag<emitter::Emitter> emitter = entity<emitter::Emitter>("/renderer/default/emitter");
            tag<sampler::Sampler> sampler = entity<sampler::Sampler>("/renderer/default/sampler");
            tag<filter::Filter> filter = entity<filter::Filter>("/renderer/default/filter");
            tag<photo::Lens> lens = entity<photo::Lens>("/renderer/default/lens");
            tag<photo::Film> film = entity<photo::Film>("/renderer/default/film");
        };
        Renderer() noexcept = default;
        Renderer(rref<Descriptor> desc) noexcept;
        auto render() noexcept -> void;
    };
}
