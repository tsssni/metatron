#pragma once
#include <metatron/render/renderer/variant.hpp>
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/core/stl/variant.hpp>
#include <glaze/glaze.hpp>

namespace mtt::renderer {
    using Emitter = stl::variant<
        emitter::Emitter,
        emitter::Uniform_Emitter
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::underlying_variant_t<mtt::renderer::Emitter>> {
        auto static constexpr tag = std::string_view{"variant"};
        auto static constexpr ids = std::array{
            "uniform",
        };
    };
}
