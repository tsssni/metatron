#include <metatron/render/scene/scene.hpp>
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/render/scene/args.hpp>
#include <metatron/render/scene/serde.hpp>
#include <metatron/render/renderer/renderer.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::scene {
    auto transform_init() noexcept -> void;
    auto spectra_init() noexcept -> void;
    auto shape_init() noexcept -> void;
    auto media_init() noexcept -> void;
    auto material_init() noexcept -> void;
    auto light_init() noexcept -> void;

    auto run() noexcept -> void {
        using namespace renderer;
        auto& hierarchy = Hierarchy::instance();
        auto& args = Args::instance();
        sampler::Sobol_Sampler::init();

        transform_init();
        spectra_init();
        shape_init();
        media_init();
        material_init();
        light_init();

        auto renderer = obj<Renderer>();
        hierarchy.filter([&renderer](auto bins){
            auto key = std::string{"renderer"};
            if (!bins.contains(key))
                stl::abort("renderer must be defined");
            auto j = bins[key].front();

            using Descriptor = Renderer::Descriptor;
            auto desc = Descriptor{};
            stl::json::load(j.serialized.str, desc);
            renderer = make_obj<Renderer>(std::move(desc));
        });
        hierarchy.populate(args.scene);
        renderer->render();
    }
}
