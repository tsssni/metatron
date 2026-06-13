#include <metatron/render/accel/hwbvh.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::accel {
    HWBVH::HWBVH(cref<Descriptor>) noexcept {
        idx = stl::vector<Divider>::storage();
    }

    auto HWBVH::operator()(
        cref<math::Ray> r, cref<fv3> n
    ) const noexcept -> opt<Interaction> {
        // GPU only
        return {};
    }
}
