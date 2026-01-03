#pragma once
#include "../command/context.hpp"
#include <metatron/device/encoder/accel.hpp>

namespace mtt::encoder {
    struct Acceleration_Encoder::Impl final {
        mut<MTL::AccelerationStructureCommandEncoder> encoder;
    };
}
