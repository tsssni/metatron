#include <metatron/render/scene/scene.hpp>
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/render/scene/args.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    auto transform_init() noexcept -> void;
    auto spectra_init() noexcept -> void;
    auto color_init() noexcept -> void;
    auto shape_init() noexcept -> void;
    auto media_init() noexcept -> void;
    auto material_init() noexcept -> void;
    auto light_init() noexcept -> void;

    auto run(i32 argc, mut<char> argv[]) noexcept -> void {
        auto& hierarchy = Hierarchy::instance();

        auto& args = Args::instance();
        args.parse(argc, argv);

        transform_init();
        spectra_init();
        color_init();
        shape_init();
        media_init();
        material_init();
        light_init();

        hierarchy.populate(args.scene);
    }
}
