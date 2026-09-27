#pragma once
#include <metatron/resource/shape/interaction.hpp>

namespace mtt::shape {
    struct Sphere final {
        struct Descriptor final {};
        Sphere(Descriptor const&) noexcept;
        Sphere() noexcept = default;

        auto size() const noexcept -> usize;
        auto bounding_box(
            math::Transform const& t, usize idx
        ) const noexcept -> math::Bounding_Box;
        auto operator()(
            math::Ray const& r, fv3 const& np,
            fv4 const& pos, usize idx = 0uz
        ) const noexcept -> Interaction;
        auto sample(
            math::Context const& ctx, fv2 const& u, usize idx = 0uz
        ) const noexcept -> Interaction;
        auto query(
            math::Ray const& r, usize idx = 0uz
        ) const noexcept -> fv4;

    private:
        u32 padding = 0u;
    };
}
