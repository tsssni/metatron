#pragma once
#include <metatron/render/accel/interaction.hpp>

namespace mtt::accel {
    struct HWBVH final {
        struct Descriptor final {};
        HWBVH(Descriptor const&) noexcept;
        HWBVH() noexcept = default;

        auto operator()(
            math::Ray const& r, fv3 const& n
        ) const noexcept -> Interaction;

    private:
        u32 idx;
    };
}
