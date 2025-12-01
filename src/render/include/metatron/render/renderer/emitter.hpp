#pragma once
#include <metatron/render/renderer/variant.hpp>
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/emitter/uniform.hpp>
#include <metatron/core/stl/variant.hpp>
#include <metatron/core/stl/json.hpp>

namespace mtt::renderer {
    using Emitter = stl::variant<
        emitter::Emitter,
        emitter::Uniform_Emitter
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::underlying_variant_t<mtt::renderer::Emitter>> {
        auto constexpr static tag = std::string_view{"variant"};
        auto constexpr static ids = std::array{
            "uniform",
        };
    };
}
