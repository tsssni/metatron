#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>

namespace mtt::render {

struct Renderer final {
    auto trace(render::Monte_Carlo_Context& ctx) noexcept -> void;
};

}
