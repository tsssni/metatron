#include <metatron/render/renderer/renderer.hpp>

namespace mtt::renderer {
    struct Renderer::Impl final {
        Descriptor desc;
        Impl(Descriptor&& desc): desc(std::move(desc)) {}
    };

    Renderer::Renderer(Descriptor&& desc) noexcept:
    stl::capsule<Renderer>(std::move(desc)) {}
}
