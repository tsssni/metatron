#pragma once
#include <metatron/render/filter/interaction.hpp>
#include <metatron/core/distribution/piecewise.hpp>

namespace mtt::render {

struct [[mtt::reflection]] Lanczos_Filter final {
    core::Piecewise_Distribution distr;
    fv2 radius;
    f32 tau;
    Lanczos_Filter(fv2 radius, f32 tau, core::recorder& recorder) noexcept;
    auto operator()(fv2 const& p) const noexcept -> f32;
    auto sample(fv2 const& u) const noexcept -> Filter_Interaction;
};

}
