#include "renderer.hpp"
#include <metatron/render/renderer/renderer.hpp>

namespace mtt::renderer {
    Renderer::Renderer(rref<Descriptor> desc) noexcept:
    stl::capsule<Renderer>(std::move(desc)) {}

    auto Renderer::render(cref<scene::Args> args) noexcept -> void {
        if (args.device == "cpu")
            return impl->trace(args);
        else if (args.device == "gpu")
            return impl->wave(args);
    }
}
