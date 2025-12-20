#include "accel.hpp"
#include "transfer.hpp"
#include "../opaque/accel.hpp"
#include "../command/buffer.hpp"

namespace mtt::encoder {
    Acceleration_Encoder::Acceleration_Encoder(
        mut<command::Buffer> cmd, mut<opaque::Acceleration> accel
    ) noexcept: cmd(cmd), accel(accel) {
        impl->load_barrier = {
            .stage = vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
            .access = vk::AccessFlagBits2::eShaderRead,
            .family = cmd->impl->family,
        };
        impl->dst_barrier = {
            .stage = vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
            .access = vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
            .family = cmd->impl->family,
        };
        impl->src_barrier = {
            .stage = vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
            .access = vk::AccessFlagBits2::eAccelerationStructureReadKHR,
            .family = cmd->impl->family,
        };
        impl->scratch_barrier = {
            .stage = vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
            .access = vk::AccessFlags2{}
            | vk::AccessFlagBits2::eAccelerationStructureReadKHR
            | vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
            .family = cmd->impl->family,
        };
        impl->use_barrier = {
            .stage = vk::PipelineStageFlagBits2::eComputeShader,
            .access = vk::AccessFlagBits2::eAccelerationStructureReadKHR,
            .family = cmd->impl->family,
        };
    }

    auto Acceleration_Encoder::build() noexcept -> void {
        auto cmd = this->cmd->impl->cmd.get();
        auto transfer = Transfer_Encoder{this->cmd};
        transfer.upload(*accel->instances);
        if (accel->bboxes) transfer.upload(*accel->bboxes);

        auto primitive_barriers = std::vector<vk::BufferMemoryBarrier2>{};
        if (accel->bboxes)
            primitive_barriers.push_back(accel->bboxes->impl->update(impl->load_barrier));
        for (auto i = 0; i < accel->buffers.size() - 1; ++i) {
            primitive_barriers.push_back(accel->buffers[i]->impl->update(impl->dst_barrier));
            primitive_barriers.push_back(accel->scratches[i]->impl->update(impl->scratch_barrier));
        }
        cmd.pipelineBarrier2({
            .bufferMemoryBarrierCount = u32(primitive_barriers.size()),
            .pBufferMemoryBarriers = primitive_barriers.data(),
        });
        cmd.buildAccelerationStructuresKHR(
            accel->impl->primitives_infos,
            accel->impl->primitvies_ptrs
        );

        auto instance_barriers = std::vector<vk::BufferMemoryBarrier2>{};
        instance_barriers.push_back(accel->instances->impl->update(impl->load_barrier));
        for (auto i = 0; i < accel->buffers.size() - 1; ++i)
            instance_barriers.push_back(accel->buffers[i]->impl->update(impl->src_barrier));
        instance_barriers.push_back(accel->buffers.back()->impl->update(impl->dst_barrier));
        instance_barriers.push_back(accel->scratches.back()->impl->update(impl->scratch_barrier));
        cmd.pipelineBarrier2({
            .bufferMemoryBarrierCount = u32(instance_barriers.size()),
            .pBufferMemoryBarriers = instance_barriers.data(),
        });
        cmd.buildAccelerationStructuresKHR(
            accel->impl->instances_info,
            accel->impl->instances_ptr
        );

        std::ranges::move(accel->scratches, std::back_inserter(this->cmd->stages));
        this->cmd->stages.push_back(std::move(accel->instances));
        if (accel->bboxes) this->cmd->stages.push_back(std::move(accel->bboxes));
    }

    auto Acceleration_Encoder::persist() noexcept -> void {
        auto cmd = this->cmd->impl->cmd.get();
        auto barriers = std::vector<vk::BufferMemoryBarrier2>{};
        for (auto i = 0; i < accel->buffers.size(); ++i)
            barriers.push_back(accel->buffers[i]->impl->update(impl->use_barrier));
        cmd.pipelineBarrier2({
            .bufferMemoryBarrierCount = u32(barriers.size()),
            .pBufferMemoryBarriers = barriers.data(),
        });
    }
}
