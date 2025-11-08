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
            Integrator integrator;
            Acceleration accel;
            Emitter emitter;
            Sampler sampler;
            Filter filter;
            Lens lens;
            photo::Film film = photo::Film::Descriptor{};
        };
        Renderer(Descriptor&& desc) noexcept;
    };
}
