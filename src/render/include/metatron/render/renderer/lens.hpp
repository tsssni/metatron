#pragma once
#include <metatron/render/renderer/variant.hpp>
#include <metatron/render/photo/lens/lens.hpp>
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/photo/lens/thin.hpp>
#include <metatron/core/stl/variant.hpp>
#include <metatron/core/stl/json.hpp>

namespace mtt::renderer {
    using Lens = stl::variant<
        photo::Lens,
        photo::Pinhole_Lens,
        photo::Thin_Lens
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::underlying_variant_t<mtt::renderer::Lens>> {
        auto constexpr static tag = std::string_view{"variant"};
        auto constexpr static ids = std::array{
            "pinhole",
            "thin",
        };
    };
}
