#include "accel.hpp"
#include "buffer.hpp"
#include <metatron/core/stl/thread.hpp>

namespace mtt::opaque {
    Acceleration::Acceleration(cref<Descriptor> desc) noexcept {
        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();

        impl->primitives.resize(desc.primitives.size());
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
        auto bboxes_data = std::vector<math::Bounding_Box>(bbox_size * num_bboxes);

        stl::scheduler::instance().sync_parallel(uzv1{desc.primitives.size()}, [&](auto idx) {
            auto [i] = idx;
            auto& prim = desc.primitives[i];
            auto procedural = prim.type == Primitive::Type::aabb;
            if (procedural) std::memcpy(
                &bboxes_data[bbox_offsets[i]],
                prim.aabbs.data(),
                prim.aabbs.size() * bbox_size
            );
        });
        bboxes = num_bboxes > 0 ? make_desc<opaque::Buffer>({
            .ptr = mut<byte>(bboxes_data.data()),
            .state = opaque::Buffer::State::local,
            .size = bbox_size * num_bboxes,
        }) : nullptr;

        auto primitives = std::vector<mut<NS::Object>>(desc.primitives.size());
        stl::scheduler::instance().sync_parallel(uzv1{desc.primitives.size()}, [&](auto idx) {
            auto [i] = idx;
            auto& prim = desc.primitives[i];
            auto procedural = prim.type == Primitive::Type::aabb;
            auto desc = mut<MTL::AccelerationStructureGeometryDescriptor>{};

            if (procedural) {
                auto besc = MTL::AccelerationStructureBoundingBoxGeometryDescriptor::alloc();
                besc->setBoundingBoxBuffer(bboxes->impl->device_buffer.get());
                besc->setBoundingBoxBufferOffset(bbox_size * bbox_offsets[i]);
                besc->setBoundingBoxStride(bbox_size);
                besc->setBoundingBoxCount(1);
                besc->setOpaque(true);
                besc->setIntersectionFunctionTableOffset(0);
                desc = besc;
            } else {
                auto tesc = MTL::AccelerationStructureTriangleGeometryDescriptor::alloc();
                tesc->setTriangleCount(prim.mesh->indices.size());
                tesc->setVertexBuffer(mut<Buffer>(prim.mesh->vertices.handle)->impl->device_buffer.get());
                tesc->setVertexBufferOffset(0);
                tesc->setVertexFormat(MTL::AttributeFormatFloat3);
                tesc->setIndexBuffer(mut<Buffer>(prim.mesh->indices.handle)->impl->device_buffer.get());
                tesc->setIndexBufferOffset(0);
                tesc->setIndexType(MTL::IndexTypeUInt32);
                tesc->setOpaque(true);
                tesc->setIntersectionFunctionTableOffset(0);
                desc = tesc;
            }

            auto pesc = MTL::PrimitiveAccelerationStructureDescriptor::alloc();
            pesc->setGeometryDescriptors(NS::Array::array(desc));
            pesc->setUsage(MTL::AccelerationStructureUsagePreferFastIntersection);
            pesc->setMotionKeyframeCount(1);
            impl->primitives[i] = device->newAccelerationStructure(pesc);
            primitives[i] = impl->primitives[i].get();
        });

        auto instances_data = std::vector<MTL::AccelerationStructureInstanceDescriptor>(desc.instances.size());
        stl::scheduler::instance().sync_parallel(uzv1{desc.instances.size()}, [&](auto idx) {
            auto [i] = idx;
            auto& instance = desc.instances[i];
            auto& info = instances_data[i];
            auto& matrix = instance.transform;
            info = MTL::AccelerationStructureInstanceDescriptor{
                .options = MTL::AccelerationStructureInstanceOptionNone,
                .mask = 0xff,
                .intersectionFunctionTableOffset = 0,
                .accelerationStructureIndex = instance.idx,
            };
            std::memcpy(&info.transformationMatrix, matrix.data(), sizeof(info.transformationMatrix));
        });
        instances = make_desc<opaque::Buffer>({
            .ptr = mut<byte>(instances_data.data()),
            .state = opaque::Buffer::State::local,
            .size = sizeof(MTL::AccelerationStructureInstanceDescriptor) * desc.instances.size(),
        });

        auto iesc = MTL::InstanceAccelerationStructureDescriptor::alloc();
        iesc->setInstancedAccelerationStructures(NS::Array::array(primitives.data(), primitives.size()));
        iesc->setInstanceCount(desc.instances.size());
        iesc->setInstanceDescriptorBuffer(instances->impl->device_buffer.get());
        iesc->setInstanceDescriptorBufferOffset(0);
        iesc->setInstanceDescriptorStride(sizeof(MTL::AccelerationStructureInstanceDescriptor));
        iesc->setInstanceTransformationMatrixLayout(MTL::MatrixLayoutRowMajor);
        iesc->setUsage(MTL::AccelerationStructureUsagePreferFastIntersection);
        impl->instances = device->newAccelerationStructure(iesc);
    }
}
