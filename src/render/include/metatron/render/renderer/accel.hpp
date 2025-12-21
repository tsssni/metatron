#pragma once
#include <metatron/render/renderer/variant.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/accel/lbvh.hpp>
#include <metatron/render/accel/hwbvh.hpp>
#include <metatron/core/stl/variant.hpp>
#include <metatron/core/stl/json.hpp>

namespace mtt::renderer {
    using Acceleration = stl::variant<
        accel::Acceleration,
        accel::LBVH,
        accel::HWBVH
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::underlying_variant_t<mtt::renderer::Acceleration>> {
        auto constexpr static tag = std::string_view{"variant"};
        auto constexpr static ids = std::array{
            "lbvh", "hwbvh"
        };
    };
}
