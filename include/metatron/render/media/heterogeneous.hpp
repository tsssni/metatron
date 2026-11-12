#pragma once
#include <metatron/render/media/henyey-greenstein.hpp>
#include <metatron/render/volume/volume.hpp>
#include <metatron/core/distribution/exponential.hpp>

namespace mtt::render {

struct [[mtt::reflection(
    sigma_a = "/spectra/zero",
    sigma_s = "/spectra/zero",
    sigma_e = "/spectra/zero"
)]] Heterogeneous_Medium final {
    Henyey_Greenstein_Phase_Function phase;
    Spectrum sigma_a;
    Spectrum sigma_s;
    Spectrum sigma_e;
    Uniform_Volume majorant;
    Volume density;
    f32 density_scale;

    Heterogeneous_Medium(
        Henyey_Greenstein_Phase_Function phase,
        Spectrum sigma_a,
        Spectrum sigma_s,
        Spectrum sigma_e,
        uzv3 dimensions,
        Volume density,
        f32 density_scale,
        core::recorder& recorder
    ) noexcept;

    struct Iterator final {
        Heterogeneous_Medium const* medium;

        fv4 sigma_a;
        fv4 sigma_s;
        fv4 sigma_e;
        fv4 sigma_maj;
        fv4 transmittance = {1.f};

        core::Ray r;

        iv3 cell;
        iv3 offset;

        f32 t_max;
        f32 t_cell;
        f32 t_boundary;
        f32 t_transmitted;
        f32 u;

        Iterator(
            Heterogeneous_Medium const& m,
            core::Ray_Context const& ctx,
            f32 t
        ) noexcept;

        auto march(f32 uu) noexcept -> Medium_Interaction<Henyey_Greenstein_Phase_Function>;

    private:
        auto update_majorant(f32 t) noexcept -> void;
        auto update_transmittance(f32 t) noexcept -> void;
    };

    auto begin(core::Ray_Context const& ctx, f32 t_max) const noexcept -> Iterator;
};

}
