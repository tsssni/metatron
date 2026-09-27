#pragma once
#include <metatron/resource/bsdf/interaction.hpp>

namespace mtt::bsdf {
    struct Interface_Bsdf final {
        struct Descriptor final {};
        Interface_Bsdf(Descriptor const&) noexcept;
        Interface_Bsdf() noexcept = default;

        auto operator()(
            fv3 const& wo, fv3 const& wi, f32 u
        ) const noexcept -> Interaction;
        auto sample(
            math::Context const& ctx, fv3 const& u
        ) const noexcept -> Interaction;
        auto flags() const noexcept -> Flags;

    private:
        u32 padding;
    };
}
