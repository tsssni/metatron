#pragma once
#include <metatron/core/math/vector.hpp>
#include <span>

namespace mtt::spectra {
    struct Discrete_Spectrum final {
        Discrete_Spectrum(std::span<f32> lambda, std::span<f32> data) noexcept;
        Discrete_Spectrum(std::span<math::Vector<f32, 2>> interleaved) noexcept;

        auto operator()(f32 lambda) const noexcept -> f32;

    private:
        std::array<f32, 256> lambda;
        std::array<f32, 256> data;
        usize size;
    };
}
