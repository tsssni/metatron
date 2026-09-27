#include <metatron/render/accel/hwbvh.hpp>

namespace mtt::accel {
    HWBVH::HWBVH(Descriptor const&) noexcept {
        idx = stl::vector<Divider>::storage();
    }

    auto HWBVH::operator()(
        math::Ray const& r, fv3 const& n
    ) const noexcept -> Interaction {
        // GPU only
        return {};
    }
}
