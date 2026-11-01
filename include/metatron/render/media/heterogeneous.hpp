#pragma once
#include <metatron/render/media/henyey-greenstein.hpp>
#include <metatron/render/volume/volume.hpp>
#include <metatron/core/distribution/exponential.hpp>

namespace mtt::render {

struct Heterogeneous_Medium final {
    render::Henyey_Greenstein_Phase_Function phase;
    render::Spectrum sigma_a;
    render::Spectrum sigma_s;
    render::Spectrum sigma_e;
    render::Uniform_Volume majorant;
    render::Volume density;
    f32 density_scale;

    Heterogeneous_Medium(
        render::Henyey_Greenstein_Phase_Function phase,
        render::Spectrum sigma_a,
        render::Spectrum sigma_s,
        render::Spectrum sigma_e,
        uzv3 dimensions,
        render::Volume density,
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

        auto march(f32 uu) noexcept -> Medium_Interaction<render::Henyey_Greenstein_Phase_Function>;

    private:
        auto update_majorant(f32 t) noexcept -> void;
        auto update_transmittance(f32 t) noexcept -> void;
    };

    auto begin(core::Ray_Context const& ctx, f32 t_max) const noexcept -> Iterator;
};

}
