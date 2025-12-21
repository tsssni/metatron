#pragma once
#include <metatron/render/accel/accel.hpp>

namespace mtt::accel {
    struct HWBVH final {
        struct Descriptor final {};
        HWBVH(cref<Descriptor> desc);

        auto operator()(
            cref<math::Ray> r, cref<fv3> n
        ) const noexcept -> opt<Interaction>;

    private:
        u32 idx;
    };
}
