#pragma once
#include <metatron/core/stl/concept.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/resource/phase/vaccum.hpp>

namespace mtt::phase {
    template<typename T>
    concept Phase_Function = requires(
        T const& t,
        fv3 const& wo, fv3 const& wi,
        math::Context const& ctx, fv2 const& u
    ) {
        { t(wo, wi) } noexcept -> stl::same_as<Interaction>;
        { t.sample(ctx, u) } noexcept -> stl::same_as<Interaction>;
    };
}
