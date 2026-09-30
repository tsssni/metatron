#pragma once
#include <metatron/render/monte-carlo/context.hpp>

namespace mtt::monte_carlo {
    struct Radiative_Integrator final {
        bool volumetric = true;

        // null scattering: https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html
        // mis method: https://pbr-book.org/4ed/Light_Transport_II_Volume_Rendering/Volume_Scattering_Integrators
        auto trace(Context& ctx) const noexcept -> void;

    private:
        struct Payload;
        template<shape::Intersectable Shape, material::Surfaced Material, media::Participating Medium>
        auto hit(Payload& payload) const noexcept -> void;
        template<shape::Intersectable Shape, material::Surfaced Material, media::Participating Medium>
        auto track(Payload& payload) const noexcept -> void;
        auto sample(Context& ctx, uzv2 const& px) const noexcept -> void;
    };
}
