#pragma once
#include <metatron/resource/material/physical.hpp>
#include <metatron/resource/material/interface.hpp>

namespace mtt::material {
    auto init() noexcept -> void;

    template<typename T>
    concept Surfaced = requires(
        T const& t,
        math::Context const& ctx,
        muldim::Coordinate const& coord
    ) {
        { t.sample(ctx, coord) } noexcept;
        requires bsdf::Scattering<decltype(t.sample(ctx, coord).bsdf)>;
        { t.flags() } noexcept -> std::same_as<Flags>;
    };

    template<typename T>
    struct Trait final: std::bool_constant<material::Surfaced<T>> {};
    using Material = stl::seal<Trait, Physical_Material, Interface_Material>;
}
