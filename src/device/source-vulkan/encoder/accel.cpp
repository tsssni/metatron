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
        impl->store_barrier = {
            .stage = vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
            .access = vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
            .family = cmd->impl->family,
        };
        impl->use_barrier = {
            .stage = vk::PipelineStageFlagBits2::eComputeShader,
            .access = vk::AccessFlagBits2::eShaderRead,
            .family = cmd->impl->family,
        };
    }

    auto Acceleration_Encoder::build() noexcept -> void {
        auto cmd = this->cmd->impl->cmd.get();
        auto transfer = Transfer_Encoder{this->cmd};
        transfer.upload(*accel->instances);
        if (accel->bboxes) transfer.upload(*accel->bboxes);

        auto barriers = std::vector<vk::BufferMemoryBarrier2>{};
        barriers.push_back(accel->instances->impl->update(impl->load_barrier));
        if (accel->bboxes)
            barriers.push_back(accel->bboxes->impl->update(impl->load_barrier));
        for (auto i = 0; i < accel->buffers.size(); ++i) {
            barriers.push_back(accel->buffers[i]->impl->update(impl->store_barrier));
            barriers.push_back(accel->scratches[i]->impl->update(impl->store_barrier));
        }
        cmd.pipelineBarrier2({
            .bufferMemoryBarrierCount = u32(barriers.size()),
            .pBufferMemoryBarriers = barriers.data(),
        });

        cmd.buildAccelerationStructuresKHR(
            accel->impl->primitives_infos,
            accel->impl->primitvies_ptrs
        );
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
