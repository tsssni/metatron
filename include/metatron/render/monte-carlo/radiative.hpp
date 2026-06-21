#pragma once
#include <metatron/render/monte-carlo/context.hpp>
#include <metatron/device/shader/argument.hpp>
#include <metatron/device/shader/pipeline.hpp>

namespace mtt::monte_carlo {
    struct Radiative_Integrator final {
        struct Descriptor final {};
        Radiative_Integrator(cref<Descriptor>) noexcept;
        Radiative_Integrator() noexcept = default;

        // null scattering: https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html
        // mis method: https://pbr-book.org/4ed/Light_Transport_II_Volume_Rendering/Volume_Scattering_Integrators
        auto acquire(cref<Context> ctx, cref<Resources> res) noexcept -> void;
        auto release() noexcept -> void;
        auto trace(ref<Context> ctx) const noexcept -> void;
        auto wave(ref<Context> ctx) const noexcept -> void;
        auto sample(ref<Ray> r) const noexcept -> opt<spectra::Stochastic_Spectrum>;

    private:
        obj<shader::Pipeline> integrate;
        obj<shader::Argument> constants;
    };
}
