#pragma once
#include <metatron/render/filter/filter.hpp>
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::renderer {
    struct Renderer final {
        stl::proxy<filter::Filter> filter;
        stl::proxy<sampler::Sampler> sampler;
    };
}
