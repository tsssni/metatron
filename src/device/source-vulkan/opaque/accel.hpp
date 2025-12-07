#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/accel.hpp>

namespace mtt::opaque {
    struct Acceleration::Impl final {
        vk::UniqueAccelerationStructureKHR tlas;
        std::vector<vk::UniqueAccelerationStructureKHR> blas;
    };
}
