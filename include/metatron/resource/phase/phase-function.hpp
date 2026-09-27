#pragma once
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/resource/phase/vaccum.hpp>

namespace mtt::phase {
    template<typename T>
    concept Phase_Function = requires(
        T const& t,
        fv3 const& wo, fv3 const& wi,
        math::Context const& ctx, fv2 const& u
    ) {
        { t(wo, wi) } noexcept -> std::same_as<Interaction>;
        { t.sample(ctx, u) } noexcept -> std::same_as<Interaction>;
    };
}
