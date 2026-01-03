#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/accel.hpp>

namespace mtt::opaque {
    struct Acceleration::Impl final {
        mtl<MTL::AccelerationStructure> instances;
        mtl<MTL::AccelerationStructureDescriptor> instances_desc;
        std::vector<mtl<MTL::AccelerationStructure>> primitives;
        std::vector<mtl<MTL::AccelerationStructureDescriptor>> primitives_descs;
    };
}
