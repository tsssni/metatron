#pragma once
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/render/sampler/independent.hpp>
#include <metatron/render/sampler/halton.hpp>
#include <metatron/render/sampler/sobol.hpp>
#include <metatron/core/stl/variant.hpp>
#include <glaze/glaze.hpp>

namespace mtt::renderer {
    using Sampler = stl::variant<
        sampler::Sampler,
        sampler::Independent_Sampler,
        sampler::Halton_Sampler,
        sampler::Sobol_Sampler
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::Sampler> {
        auto static constexpr tag = std::string_view{"variant"};
        auto static constexpr ids = std::array{
            "independent",
            "halton",
            "sobol",
        };
    };
}
