#include <metatron/render/accel/accel.hpp>
#include <metatron/resource/serde/serde.hpp>

namespace mtt::accel {
    auto init() noexcept -> void {
        MTT_DESERIALIZE(Divider);
        MTT_DESERIALIZE(LBVH);
    }
}
