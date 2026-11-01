#pragma once
#include <metatron/render/shape/interaction.hpp>

namespace mtt::render {

struct Sphere final {
    auto size() const noexcept -> usize;
    auto bounding_box(
        core::Transform const& t, usize idx
    ) const noexcept -> core::Bounding_Box;
    auto operator()(
        core::Ray const& r, fv3 const& np,
        fv4 const& pos, usize idx = 0uz
    ) const noexcept -> Shape_Interaction;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u, usize idx = 0uz
    ) const noexcept -> Shape_Interaction;
    auto query(
        core::Ray const& r, usize idx = 0uz
    ) const noexcept -> fv4;
};

}
