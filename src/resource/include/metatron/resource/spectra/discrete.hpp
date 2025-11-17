#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::spectra {
    struct Discrete_Spectrum final {
        struct Descriptor final {
            std::string path;
        };
        Discrete_Spectrum() noexcept = default;
        Discrete_Spectrum(cref<Descriptor> desc) noexcept;
        auto operator()(f32 lambda) const noexcept -> f32;

    private:
        buf<f32> lambda;
        buf<f32> storage;
    };
}
