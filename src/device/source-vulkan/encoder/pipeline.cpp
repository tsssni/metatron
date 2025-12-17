#include "pipeline.hpp"
#include "../shader/pipeline.hpp"
#include "../command/buffer.hpp"

namespace mtt::encoder {
    Pipeline_Encoder::Pipeline_Encoder(
        mut<command::Buffer> cmd, mut<shader::Pipeline> ppl
    ) noexcept: cmd(cmd), ppl(ppl) {
        impl->barrier = opaque::Barrier{
            .stage = vk::PipelineStageFlagBits2::eComputeShader,
            .access = vk::AccessFlagBits2::eDescriptorBufferReadEXT,
            .family = cmd->impl->family,
        };
    }

    auto Pipeline_Encoder::bind() noexcept -> void {
        auto cmd = this->cmd->impl->cmd.get();
        auto barriers = std::vector<vk::BufferMemoryBarrier2>(ppl->args.size());
        auto infos = std::vector<vk::DescriptorBufferBindingInfoEXT>(ppl->args.size());
        for (auto i = 0; i < infos.size(); ++i) {
            auto args = ppl->args[i];
            barriers[i] = args->set->impl->update(impl->barrier);
            infos[i] = vk::DescriptorBufferBindingInfoEXT{
                .address = ppl->impl->sets[i],
                .usage = vk::BufferUsageFlags{}
                | vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT
                | vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT,
            };
        }
        cmd.pipelineBarrier2({
            .bufferMemoryBarrierCount = u32(barriers.size()),
            .pBufferMemoryBarriers = barriers.data(),
        });
        cmd.bindDescriptorBuffersEXT(infos);

        auto indices = std::views::iota(0u, u32(ppl->args.size()))
        | std::ranges::to<std::vector<u32>>();
        auto offsets = std::vector<usize>(ppl->args.size(), 0);
        cmd.setDescriptorBufferOffsets2EXT(vk::SetDescriptorBufferOffsetsInfoEXT{
            .stageFlags = vk::ShaderStageFlagBits::eCompute,
            .layout = ppl->impl->layout.get(),
            .firstSet = 0,
            .setCount = u32(ppl->args.size()),
            .pBufferIndices = indices.data(),
            .pOffsets = offsets.data(),
        });
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, ppl->impl->pipeline.get());
    }
}
