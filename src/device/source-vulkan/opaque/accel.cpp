#include "accel.hpp"
#include "buffer.hpp"
#include <metatron/core/stl/thread.hpp>

namespace mtt::opaque {
    Acceleration::Acceleration(cref<Descriptor> desc) noexcept {
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();

        impl->primitives.resize(desc.primitives.size());
        impl->primitives_geometries.resize(desc.primitives.size());
        impl->primitives_infos.resize(desc.primitives.size());
        impl->primitvies_ranges.resize(desc.primitives.size());
        impl->primitvies_ptrs.resize(desc.primitives.size());
        buffers.resize(desc.primitives.size() + 1);
        scratches.resize(desc.primitives.size() + 1);

        auto bbox_size = sizeof(math::Bounding_Box);
        auto bbox_offsets = std::vector<usize>(desc.primitives.size() + 1, 0);
        auto num_bboxes = 0;
        for (auto i = 0; i < desc.primitives.size(); ++i)
            if (desc.primitives[i].type == Primitive::Type::aabb) {
                bbox_offsets[i + 1] = bbox_offsets[i] + desc.primitives[i].aabbs.size();
                num_bboxes += desc.primitives[i].aabbs.size();
            } else bbox_offsets[i + 1] = bbox_offsets[i];
        bboxes = num_bboxes > 0 ? make_obj<opaque::Buffer>(
        opaque::Buffer::Descriptor{
            .state = opaque::Buffer::State::twin,
            .size = bbox_size * num_bboxes,
            .flags = u64(vk::BufferUsageFlagBits2::eAccelerationStructureBuildInputReadOnlyKHR),
        }) : nullptr;

        stl::scheduler::instance().sync_parallel(uzv1{desc.primitives.size()}, [&](auto idx) {
            auto [i] = idx;
            auto& prim = desc.primitives[i];
            auto procedural = prim.type == Primitive::Type::aabb;
            if (procedural) {
                auto offset = bbox_size * bbox_offsets[i];
                auto size = prim.aabbs.size() * bbox_size;
                std::memcpy(bboxes->ptr + offset, prim.aabbs.data(), size);
                bboxes->dirty.push_back({offset, size});
            }

            impl->primitives_geometries[i] = {
                .geometryType = procedural
                ? vk::GeometryTypeKHR::eAabbs
                : vk::GeometryTypeKHR::eTriangles,
                .geometry = procedural
                ? vk::AccelerationStructureGeometryDataKHR{.aabbs = {
                    .data = {.deviceAddress = bboxes->addr + bbox_size * bbox_offsets[i]},
                    .stride = bbox_size,
                }}
                : vk::AccelerationStructureGeometryDataKHR{.triangles = {
                    .vertexFormat = vk::Format::eR32G32B32Sfloat,
                    .vertexData = {.deviceAddress = uptr(prim.mesh->vertices.ptr)},
                    .vertexStride = sizeof(fv3),
                    .maxVertex = u32(prim.mesh->vertices.size()) - 1,
                    .indexType = vk::IndexType::eUint32,
                    .indexData = {.deviceAddress = uptr(prim.mesh->indices.ptr)}
                }},
                .flags = vk::GeometryFlagBitsKHR::eOpaque,
            };
            impl->primitives_infos[i] = {
                .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
                .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
                .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
                .geometryCount = 1,
                .pGeometries = &impl->primitives_geometries[i],
            };
            impl->primitvies_ranges[i] = {
                .primitiveCount = u32(procedural ? prim.aabbs.size() : prim.mesh->indices.size()),
                .primitiveOffset = 0,
                .firstVertex = 0,
                .transformOffset = 0,
            };
            impl->primitvies_ptrs[i] = &impl->primitvies_ranges[i];

            auto size = device.getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice,
                impl->primitives_infos[i],
                procedural ? prim.aabbs.size() : prim.mesh->indices.size()
            );
            buffers[i] = make_obj<opaque::Buffer>(
            opaque::Buffer::Descriptor{
                .state = opaque::Buffer::State::local,
                .type = desc.type,
                .size = size.accelerationStructureSize,
                .flags = u64(vk::BufferUsageFlagBits2::eAccelerationStructureStorageKHR),
            });
            scratches[i] = make_obj<opaque::Buffer>(
            opaque::Buffer::Descriptor{
                .state = opaque::Buffer::State::local,
                .type = desc.type,
                .size = size.buildScratchSize,
                .flags = u64(vk::BufferUsageFlagBits2::eStorageBuffer),
            });

            impl->primitives[i] = command::guard(device.createAccelerationStructureKHRUnique({
                .buffer = buffers[i]->impl->device_buffer.get(),
                .offset = 0,
                .size = size.accelerationStructureSize,
                .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
            }));
            impl->primitives_infos[i].dstAccelerationStructure = impl->primitives[i].get();
            impl->primitives_infos[i].scratchData = {.deviceAddress = scratches[i]->addr};
        });

        instances = make_obj<opaque::Buffer>(
        opaque::Buffer::Descriptor{
            .state = opaque::Buffer::State::twin,
            .size = sizeof(vk::AccelerationStructureInstanceKHR) * desc.instances.size(),
            .flags = u64(vk::BufferUsageFlagBits2::eAccelerationStructureBuildInputReadOnlyKHR),
        });
        stl::scheduler::instance().sync_parallel(uzv1{desc.instances.size()}, [&](auto idx) {
            auto [i] = idx;
            auto& instance = desc.instances[i];
            auto size = sizeof(vk::AccelerationStructureInstanceKHR);
            auto info = vk::AccelerationStructureInstanceKHR{
                .accelerationStructureReference = device.getAccelerationStructureAddressKHR({
                    .accelerationStructure = impl->primitives[instance.idx].get(),
                })
            };
            std::memcpy(info.transform.matrix.data(), instance.transform.data(), sizeof(f32) * 12);
            std::memcpy(instances->ptr + i * size, &info, size);
        });

        impl->instances_geometry = {
            .geometryType = vk::GeometryTypeKHR::eInstances,
            .geometry = {.instances{
                .arrayOfPointers = false,
                .data = {.deviceAddress = instances->addr},
            }},
            .flags = vk::GeometryFlagBitsKHR::eOpaque,
        };
        impl->instances_info = {
            .type = vk::AccelerationStructureTypeKHR::eTopLevel,
            .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
            .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
            .geometryCount = 1,
            .pGeometries = &impl->instances_geometry,
        };
        impl->instances_range = {
            .primitiveCount = u32(desc.instances.size()),
            .primitiveOffset = 0,
            .firstVertex = 0,
            .transformOffset = 0,
        };
        impl->instances_ptr = &impl->instances_range;

        auto size = device.getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice,
            impl->instances_info,
            desc.instances.size()
        );
        buffers.back() = make_obj<opaque::Buffer>(
        opaque::Buffer::Descriptor{
            .state = opaque::Buffer::State::local,
            .type = desc.type,
            .size = size.accelerationStructureSize,
            .flags = u64(vk::BufferUsageFlagBits2::eAccelerationStructureStorageKHR),
        });
        scratches.back() = make_obj<opaque::Buffer>(
        opaque::Buffer::Descriptor{
            .state = opaque::Buffer::State::local,
            .type = desc.type,
            .size = size.buildScratchSize,
            .flags = u64(vk::BufferUsageFlagBits2::eStorageBuffer),
        });

        impl->instances = command::guard(device.createAccelerationStructureKHRUnique({
            .buffer = buffers.back()->impl->device_buffer.get(),
            .offset = 0,
            .size = size.accelerationStructureSize,
            .type = vk::AccelerationStructureTypeKHR::eTopLevel,
        }));
        impl->instances_info.dstAccelerationStructure = impl->instances.get();
        impl->instances_info.scratchData = {.deviceAddress = scratches.back()->addr};
    }
}
