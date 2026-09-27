#pragma once
#include <metatron/render/photo/film.hpp>
#include <metatron/render/photo/lens/lens.hpp>
#include <metatron/render/sampler/sampler.hpp>

namespace mtt::photo {
    struct Interaction final {
        math::Ray_Differential ray_differential;
        f32 pdf;
    };

    struct Camera final {
        auto sample(
            Lens lens,
            fv2 const& pos,
            fv2 const& dxdy,
            fv2 const& u
        ) noexcept -> Interaction;
    };
}
