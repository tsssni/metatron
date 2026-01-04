#include "pipeline.hpp"
#include "../shader/pipeline.hpp"
#include "../opaque/buffer.hpp"
#include "../command/buffer.hpp"

namespace mtt::encoder {
    Pipeline_Encoder::Pipeline_Encoder(
        mut<command::Buffer> cmd, mut<shader::Pipeline> ppl
    ) noexcept: cmd(cmd), ppl(ppl) {
        impl->encoder = cmd->impl->cmd->computeCommandEncoder();
        impl->encoder->waitForFence(cmd->impl->fence.get());
    }

    auto Pipeline_Encoder::submit() noexcept -> void {
        impl->encoder->updateFence(cmd->impl->fence.get());
        impl->encoder->endEncoding();
    }

    auto Pipeline_Encoder::bind() noexcept -> void {
        impl->encoder->setComputePipelineState(ppl->impl->pipeline.get());
        auto buffers = ppl->args
        | std::views::transform([](auto x) { return x->set->impl->device_buffer.get(); })
        | std::ranges::to<std::vector<mut<MTL::Buffer>>>();
        auto offsets = std::vector<usize>(ppl->args.size(), 0uz);
        impl->encoder->setBuffers(buffers.data(), offsets.data(), {0, u32(buffers.size())});
    }

    auto Pipeline_Encoder::dispatch(uv3 threads, uv3 group) noexcept -> void {
        impl->encoder->dispatchThreads(
            {threads[0], threads[1], threads[2]},
            {group[0], group[1], group[2]}
        );
    }
}
