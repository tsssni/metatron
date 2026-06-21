#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>

namespace mtt::renderer {
    struct Renderer final {
        auto trace() noexcept -> void;
        auto wave() noexcept -> void;
    };
}
