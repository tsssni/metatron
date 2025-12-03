#pragma once
#include <metatron/render/renderer/variant.hpp>
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/render/sampler/independent.hpp>
#include <metatron/render/sampler/halton.hpp>
#include <metatron/render/sampler/sobol.hpp>
#include <metatron/core/stl/variant.hpp>
#include <metatron/core/stl/json.hpp>

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
    struct meta<mtt::renderer::underlying_variant_t<mtt::renderer::Sampler>> {
        auto constexpr static tag = std::string_view{"variant"};
        auto constexpr static ids = std::array{
            "independent",
            "halton",
            "sobol",
        };
    };
}
