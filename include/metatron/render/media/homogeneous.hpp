#pragma once
#include <metatron/render/media/henyey-greenstein.hpp>

namespace mtt::render {

struct Homogeneous_Medium final {
    render::Henyey_Greenstein_Phase_Function phase;
    render::Spectrum sigma_a;
    render::Spectrum sigma_s;
    render::Spectrum sigma_e;

    struct Iterator final {
        Homogeneous_Medium const* medium;
        core::Ray r;
        fv4 lambda;
        f32 t_max;
        auto march(f32 u) noexcept -> Medium_Interaction<render::Henyey_Greenstein_Phase_Function>;
    };

    auto begin(
        core::Ray_Context const& ctx, f32 t_max
    ) const noexcept -> Iterator;
};

}
