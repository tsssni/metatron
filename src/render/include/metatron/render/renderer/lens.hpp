#pragma once
#include <metatron/render/photo/lens/lens.hpp>
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/photo/lens/thin.hpp>
#include <metatron/core/stl/variant.hpp>
#include <glaze/glaze.hpp>

namespace mtt::renderer {
    using Lens = stl::variant<
        photo::Lens,
        photo::Pinhole_Lens,
        photo::Thin_Lens
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::Lens> {
        auto static constexpr tag = std::string_view{"variant"};
        auto static constexpr ids = std::array{
            "pinhole",
            "thin",
        };
    };
}
