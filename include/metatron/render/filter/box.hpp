#pragma once
#include <metatron/render/filter/interaction.hpp>

namespace mtt::filter {
    struct Box_Filter final {
        struct Descriptor final {
            fv2 radius = {0.5f};
        };
        Box_Filter(Descriptor const& desc) noexcept;
        Box_Filter() noexcept = default;
        auto operator()(fv2 const& p) const noexcept -> f32;
        auto sample(fv2 const& u) const noexcept -> Interaction;

    private:
        fv2 radius;
    };
}
