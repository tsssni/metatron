#pragma once
#include <metatron/resource/phase/phase-function.hpp>
#include <metatron/resource/spectra/spectrum.hpp>

namespace mtt::media {
    struct Interaction final {
        phase::Phase_Function phase;
        fv3 p;
        f32 t;
        fv4 transmittance;
        fv4 sigma_a;
        fv4 sigma_s;
        fv4 sigma_n;
        fv4 sigma_maj;
        fv4 sigma_e;
    };

    struct Phase final {
        enum struct Function {
            henyey_greenstein,
        } function = Function::henyey_greenstein;
        f32 g = 0.f;

        auto to_phase() const noexcept -> phase::Phase_Function;
    };
}
