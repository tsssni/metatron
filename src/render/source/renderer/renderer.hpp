#pragma once
#include <metatron/render/renderer/renderer.hpp>

namespace mtt::renderer {
    struct Renderer::Impl final {
        Descriptor desc;
        Impl(rref<Descriptor> desc) noexcept;
        auto trace() noexcept -> void;
        auto wave() noexcept -> void;
    };
}
