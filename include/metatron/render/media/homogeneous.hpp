#pragma once
#include <metatron/render/media/henyey-greenstein.hpp>

namespace mtt::render {

struct [[mtt::reflection(
    sigma_a = "/spectra/zero",
    sigma_s = "/spectra/zero",
    sigma_e = "/spectra/zero"
)]] Homogeneous_Medium final {
    Henyey_Greenstein_Phase_Function phase;
    Spectrum sigma_a;
    Spectrum sigma_s;
    Spectrum sigma_e;

    struct Iterator final {
        Homogeneous_Medium const* medium;
        core::Ray r;
        fv4 lambda;
        f32 t_max;
        auto march(f32 u) noexcept -> Medium_Interaction<Henyey_Greenstein_Phase_Function>;
    };

    auto begin(
        core::Ray_Context const& ctx, f32 t_max
    ) const noexcept -> Iterator;
};

}
