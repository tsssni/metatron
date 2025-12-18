#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/accel.hpp>

namespace mtt::opaque {
    struct Acceleration::Impl final {
        vk::UniqueAccelerationStructureKHR instances;
        vk::AccelerationStructureGeometryKHR instances_geometry;
        vk::AccelerationStructureBuildGeometryInfoKHR instances_info;
        vk::AccelerationStructureBuildRangeInfoKHR instances_range;
        std::vector<vk::UniqueAccelerationStructureKHR> primitives;
        std::vector<vk::AccelerationStructureGeometryKHR> primitives_geometries;
        std::vector<vk::AccelerationStructureBuildGeometryInfoKHR> primitives_infos;
        std::vector<vk::AccelerationStructureBuildRangeInfoKHR> primitvies_ranges;
    };
}
