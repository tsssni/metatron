#pragma once
#include <metatron/render/renderer/integrator.hpp>
#include <metatron/render/renderer/accel.hpp>
#include <metatron/render/renderer/emitter.hpp>
#include <metatron/render/renderer/sampler.hpp>
#include <metatron/render/renderer/filter.hpp>
#include <metatron/render/renderer/lens.hpp>
#include <metatron/render/photo/film.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::renderer {
    struct Renderer final: stl::capsule<Renderer> {
        struct Impl;
        struct Descriptor final {
            photo::Film film = photo::Film::Descriptor{};
            Integrator integrator = monte_carlo::Volume_Path_Integrator{};
            Acceleration accel = accel::LBVH{{}};
            Emitter emitter = emitter::Uniform_Emitter{};
            Sampler sampler = sampler::Sobol_Sampler{film.spp, film.image->size};
            Filter filter = filter::Lanczos_Filter{};
            Lens lens = photo::Thin_Lens{{}};
        };
        Renderer() noexcept = default;
        Renderer(Descriptor&& desc) noexcept;
        auto render() noexcept -> void;
    };
}
