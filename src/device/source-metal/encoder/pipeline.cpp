#include "pipeline.hpp"
#include "../shader/pipeline.hpp"
#include "../opaque/buffer.hpp"
#include "../command/buffer.hpp"

namespace mtt::encoder {
    Pipeline_Encoder::Pipeline_Encoder(
        mut<command::Buffer> cmd, mut<shader::Pipeline> ppl
    ) noexcept: cmd(cmd), ppl(ppl) { impl->encoder = cmd->impl->cmd->computeCommandEncoder(); }
    auto Pipeline_Encoder::submit() noexcept -> void { impl->encoder->endEncoding(); }

    auto Pipeline_Encoder::bind() noexcept -> void {
        impl->encoder->setComputePipelineState(ppl->impl->pipeline.get());
        auto buffers = ppl->args
        | std::views::transform([](auto x) { return x->set->impl->device_buffer.get(); })
        | std::ranges::to<std::vector<mut<MTL::Buffer>>>();
        auto offsets = std::vector<usize>(ppl->args.size(), 0uz);
        impl->encoder->setBuffers(buffers.data(), offsets.data(), {0, u32(buffers.size())});
    }

    auto Pipeline_Encoder::dispatch(uzv3 grid) noexcept -> void {
        auto [x, y, z] = grid;
        impl->encoder->dispatchThreadgroups({x, y, z}, {8, 8, 1});
    }
}
