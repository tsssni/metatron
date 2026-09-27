#pragma once
#include <metatron/render/monte-carlo/context.hpp>

namespace mtt::monte_carlo {
    struct Radiative_Integrator final {
        struct Descriptor final {};
        Radiative_Integrator(Descriptor const&) noexcept;
        Radiative_Integrator() noexcept = default;

        // null scattering: https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html
        // mis method: https://pbr-book.org/4ed/Light_Transport_II_Volume_Rendering/Volume_Scattering_Integrators
        auto trace(Context& ctx) const noexcept -> void;

    private:
        struct Payload;
        auto hit(Payload& payload) const noexcept -> void;
        auto track(Payload& payload, accel::Acceleration const& accel) const noexcept -> void;
        auto sample(Context& ctx, uzv2 const& px) const noexcept -> void;
    };
}
