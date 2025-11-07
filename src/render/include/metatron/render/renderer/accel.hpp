#pragma once
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/accel/lbvh.hpp>
#include <metatron/core/stl/variant.hpp>
#include <glaze/glaze.hpp>

namespace mtt::renderer {
    using Acceleration = stl::variant<
        accel::Acceleration,
        accel::LBVH
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::Acceleration> {
        auto static constexpr tag = std::string_view{"variant"};
        auto static constexpr ids = std::array{
            "lbvh",
        };
    };
}
