#include "queue.hpp"
#include "buffer.hpp"
#include "timeline.hpp"
#include <metatron/core/stl/thread.hpp>

namespace mtt::command {
    Queue::Queue(Type type) noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        impl->queue = device->newCommandQueue();
        auto size = stl::scheduler::instance().size();
        impl->cmds.resize(size);
    }
    Queue::~Queue() noexcept {}

    auto Queue::allocate() noexcept -> obj<Buffer> {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        auto idx = stl::scheduler::index();
        auto& cmds = impl->cmds[idx];

        auto temp = std::deque<obj<Buffer>>();
        auto picked = obj<Buffer>{};
        while (!cmds.empty()) {
            auto finished = true;
            for (auto [timeline, count]: cmds.front()->signals)
                finished &= timeline->wait(count, 0);
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

        if (picked) {
            picked->impl->cmd = impl->queue->commandBufferWithUnretainedReferences();
            picked->blocks.clear();
            picked->stages.clear();
            picked->waits.clear();
            picked->signals.clear();
            return picked;
        } else {
            auto cmd = make_obj<Buffer>();
            cmd->type = type;
            cmd->blocks.cmd = cmd.get();
            cmd->impl->cmd = impl->queue->commandBufferWithUnretainedReferences();
            return cmd;
        }
        return make_obj<Buffer>();
    }

    auto Queue::submit(rref<obj<Buffer>> cmd) noexcept -> void {
        for (auto [timeline, count]: cmd->waits)
            cmd->impl->cmd->encodeWait(timeline->impl->event.get(), count);
        for (auto [timeline, count]: cmd->signals)
            cmd->impl->cmd->encodeSignalEvent(timeline->impl->event.get(), count);
        cmd->impl->cmd->commit();
        auto& cmds = impl->cmds[stl::scheduler::index()];
        cmds.push_back(std::move(cmd));
    }
}
