#pragma once
#include "../opaque/buffer.hpp"
#include <metatron/device/encoder/accel.hpp>

namespace mtt::encoder {
    struct Acceleration_Encoder::Impl final {
        opaque::Barrier load_barrier;
        opaque::Barrier dst_barrier;
        opaque::Barrier src_barrier;
        opaque::Barrier scratch_barrier;
        opaque::Barrier use_barrier;
    };
}
