#pragma once
#include <metatron/render/renderer/variant.hpp>
#include <metatron/render/monte-carlo/integrator.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/core/stl/variant.hpp>
#include <metatron/core/stl/json.hpp>

namespace mtt::renderer {
    using Integrator = stl::variant<
        monte_carlo::Integrator,
        monte_carlo::Volume_Path_Integrator
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::underlying_variant_t<mtt::renderer::Integrator>> {
        auto constexpr static tag = std::string_view{"variant"};
        auto constexpr static ids = std::array{
            "volume_path",
        };
    };
}
