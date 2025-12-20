#include "renderer.hpp"
#include <metatron/render/renderer/renderer.hpp>
#include <metatron/render/scene/args.hpp>

namespace mtt::renderer {
    Renderer::Renderer(rref<Descriptor> desc) noexcept:
    stl::capsule<Renderer>(std::move(desc)) {}

    auto Renderer::render() noexcept -> void {
        auto& args = scene::Args::instance();
        if (args.device == "cpu")
            return impl->trace();
        else if (args.device == "gpu")
            return impl->wave();
    }
}
