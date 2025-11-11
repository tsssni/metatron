#pragma once
#include <metatron/render/renderer/variant.hpp>
#include <metatron/render/filter/filter.hpp>
#include <metatron/render/filter/box.hpp>
#include <metatron/render/filter/gaussian.hpp>
#include <metatron/render/filter/lanczos.hpp>
#include <metatron/core/stl/variant.hpp>
#include <glaze/glaze.hpp>

namespace mtt::renderer {
    using Filter = stl::variant<
        filter::Filter,
        filter::Box_Filter,
        filter::Gaussian_Filter,
        filter::Lanczos_Filter
    >;
};

namespace glz {
    template<>
    struct meta<mtt::renderer::underlying_variant_t<mtt::renderer::Filter>> {
        auto static constexpr tag = std::string_view{"variant"};
        auto static constexpr ids = std::array{
            "box",
            "gaussian",
            "lanczos",
        };
    };
}
