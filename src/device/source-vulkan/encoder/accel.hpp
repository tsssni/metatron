#pragma once
#include "../opaque/buffer.hpp"
#include <metatron/device/encoder/accel.hpp>

namespace mtt::encoder {
    struct Acceleration_Encoder::Impl final {
        opaque::Barrier load_barrier;
        opaque::Barrier store_barrier;
        opaque::Barrier use_barrier;
    };
}
