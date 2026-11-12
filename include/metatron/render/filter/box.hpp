#pragma once
#include <metatron/render/filter/interaction.hpp>

namespace mtt::render {

struct [[mtt::reflection]] Box_Filter final {
    fv2 radius;
    auto operator()(fv2 const& p) const noexcept -> f32;
    auto sample(fv2 const& u) const noexcept -> Filter_Interaction;
};

}
