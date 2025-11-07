#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>
#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/core/stl/variant.hpp>
#include <glaze/glaze.hpp>

namespace mtt::renderer {
    using Integrator = stl::variant<
        monte_carlo::Integrator,
        monte_carlo::Volume_Path_Integrator
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::Integrator> {
        auto static constexpr tag = std::string_view{"variant"};
        auto static constexpr ids = std::array{
            "volume_path",
        };
    };
}
