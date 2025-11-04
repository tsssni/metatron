#include <metatron/render/scene/scene.hpp>
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    auto spectra_init() noexcept -> void;
    auto color_init() noexcept -> void;
    auto shape_init() noexcept -> void;
    auto media_init() noexcept -> void;
    auto material_init() noexcept -> void;
    auto light_init() noexcept -> void;

    auto init() noexcept -> void {
        spectra_init();
        color_init();
        shape_init();
        media_init();
        material_init();
        light_init();
    }

    auto test() noexcept -> void {
    }
}
