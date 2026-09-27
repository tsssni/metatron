#include "queue.hpp"
#include "buffer.hpp"
#include "timeline.hpp"
#include <metatron/core/stl/thread.hpp>

namespace mtt::command {
    Queue::Queue(Type type) noexcept {
        auto& ctx = Context::internal();
        auto size = stl::scheduler::size();
        impl->queue = ctx->device->newCommandQueue(size * 8);
        impl->queue->addResidencySet(ctx->residency.get());
        impl->cmds.resize(size);
    }
    Queue::~Queue() noexcept {}

    auto Queue::allocate(Pairs&& waits) noexcept -> std::unique_ptr<Buffer> {
        auto& ctx = Context::internal();
        auto device = ctx->device.get();
        auto idx = stl::scheduler::index();
        auto& cmds = impl->cmds[idx];

        auto temp = std::deque<std::unique_ptr<Buffer>>();
        auto picked = std::unique_ptr<Buffer>{};
        while (!cmds.empty()) {
            auto finished = cmds.front()->impl->cmd->status() == MTL::CommandBufferStatusCompleted;
            auto front = std::move(cmds.front());
            cmds.pop_front();
            if (finished) {
                picked = std::move(front);
                break;
            } else temp.push_back(std::move(front));
        }

        while (!temp.empty()) {
            cmds.push_back(std::move(temp.front()));
            temp.pop_front();
        }
        auto allocated = impl->queue->commandBuffer();
        for (auto [timeline, count]: waits)
            allocated->encodeWait(timeline->impl->event.get(), count);

        if (picked) {
            picked->blocks.clear();
            picked->stages.clear();
            picked->waits.clear();
            picked->signals.clear();
            picked->impl->cmd = allocated;
            return picked;
        } else {
            auto cmd = std::make_unique<Buffer>();
            cmd->type = type;
            cmd->blocks.cmd = cmd.get();
            cmd->impl->cmd = allocated;
            cmd->impl->fence = device->newFence();
            return cmd;
        }
    }

    auto Queue::submit(std::unique_ptr<Buffer>&& cmd, Pairs&& signals) noexcept -> void {
        for (auto [timeline, count]: signals)
            cmd->impl->cmd->encodeSignalEvent(timeline->impl->event.get(), count);
        cmd->impl->cmd->commit();
        auto& cmds = impl->cmds[stl::scheduler::index()];
        cmds.push_back(std::move(cmd));
    }
}
