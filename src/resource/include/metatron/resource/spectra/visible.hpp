#pragma once
#include <metatron/resource/spectra/spectrum.hpp>

namespace mtt::spectra {
    auto constexpr visible_range = usize(visible_lambda[1] - visible_lambda[0] + 1.f);

    struct Visible_Spectrum final {
        struct Descriptor final {
            std::string path;
        };
        Visible_Spectrum(cref<Descriptor> desc) noexcept;
        Visible_Spectrum() noexcept = default;
        auto operator()(f32 lambda) const noexcept -> f32;

    private:
        std::array<f32, visible_range> storage;
    };
}
