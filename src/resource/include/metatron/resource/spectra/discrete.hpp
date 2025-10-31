#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::spectra {
    struct Discrete_Spectrum final {
        struct Descriptor final {
            std::string path;
        };
        Discrete_Spectrum(Descriptor const& desc) noexcept;
        auto operator()(f32 lambda) const noexcept -> f32;

    private:
        std::array<f32, 256> lambda;
        std::array<f32, 256> storage;
        usize size;
    };
}
