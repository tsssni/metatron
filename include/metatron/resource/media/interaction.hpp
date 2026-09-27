#pragma once
#include <metatron/resource/phase/phase-function.hpp>
#include <metatron/resource/spectra/spectrum.hpp>

namespace mtt::media {
    template<phase::Phase_Function Phase_Function>
    struct Interaction final {
        Phase_Function phase;
        fv3 p;
        f32 t;
        fv4 transmittance;
        fv4 sigma_a;
        fv4 sigma_s;
        fv4 sigma_n;
        fv4 sigma_maj;
        fv4 sigma_e;
    };
}
