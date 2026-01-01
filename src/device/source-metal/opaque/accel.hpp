#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/accel.hpp>

namespace mtt::opaque {
    struct Acceleration::Impl final {
        mtl<MTL::AccelerationStructure> instances;
        std::vector<mtl<MTL::AccelerationStructure>> primitives;
    };
}
