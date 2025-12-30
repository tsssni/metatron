#pragma once
#include <metatron/render/accel/accel.hpp>

namespace mtt::accel {
    struct HWBVH final {
        struct Descriptor final {};
        HWBVH(cref<Descriptor>) noexcept;
        HWBVH() noexcept = default;

        auto operator()(
            cref<math::Ray> r, cref<fv3> n
        ) const noexcept -> opt<Interaction>;

    private:
        u32 idx;
    };
}
