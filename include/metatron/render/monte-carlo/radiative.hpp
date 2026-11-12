#pragma once

namespace mtt::render {

struct Monte_Carlo_Context;

struct [[mtt::reflection]] Radiative_Integrator final {
    bool volumetric = true;

    // null scattering: https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html
    // mis method: https://pbr-book.org/4ed/Light_Transport_II_Volume_Rendering/Volume_Scattering_Integrators
    auto trace(core::recorder& r, Monte_Carlo_Context& ctx) const noexcept -> void;
};

}
