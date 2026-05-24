#pragma once
#include <metatron/render/renderer/renderer.hpp>

namespace mtt::renderer {
    struct Renderer::Impl final {
        Descriptor desc;
        auto trace(cref<scene::Args> args) noexcept -> void;
        auto wave(cref<scene::Args> args) noexcept -> void;
    };
}
