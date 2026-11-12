#pragma once
#include <metatron/render/light/interaction.hpp>
#include <metatron/render/shape/shape.hpp>

namespace mtt::render {

struct [[mtt::reflection]] Area_Light final {
    Shape shape;
    u32 primitive;

    auto operator()(
        core::Ray const& r, fv4 const& lambda
    ) const noexcept -> Light_Interaction;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> Light_Interaction;
    auto flags() const noexcept -> Light_Flags;
};

}
