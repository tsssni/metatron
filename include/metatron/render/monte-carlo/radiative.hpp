#pragma once
#include <metatron/render/monte-carlo/context.hpp>

namespace mtt::monte_carlo {
    struct Radiative_Integrator final {
        struct Descriptor final {};
        Radiative_Integrator(cref<Descriptor>) noexcept;
        Radiative_Integrator() noexcept = default;

        // null scattering: https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html
        // mis method: https://pbr-book.org/4ed/Light_Transport_II_Volume_Rendering/Volume_Scattering_Integrators
        auto trace(ref<Context> ctx) const noexcept -> void;

    private:
        struct Payload;
        auto hit(ref<Payload> payload) const noexcept -> void;
        auto track(ref<Payload> payload, cref<accel::Acceleration> accel) const noexcept -> void;
        auto sample(ref<Context> ctx, cref<uzv2> px) const noexcept -> void;
    };
}
