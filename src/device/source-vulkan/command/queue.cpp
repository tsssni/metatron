#include "queue.hpp"
#include "buffer.hpp"
#include "timeline.hpp"
#include <metatron/core/stl/thread.hpp>
#include <atomic>

namespace mtt::command {
    std::array<Family, num_types> Queue::Impl::families;

    Queue::Queue(Type type) noexcept: type(type) {
        auto& ctx = Context::instance().impl;
        auto& family = impl->families[u32(type)];
        auto device = ctx->device.get();
        auto idx = family.allocated;
        if (idx >= family.count) stl::abort("queue family {} overflow", u32(type));
        ++family.allocated;

        impl->queue = device.getQueue2({
            .queueFamilyIndex = family.idx,
            .queueIndex = idx,
        });
        auto size = stl::scheduler::instance().size();
        impl->pools.resize(size);
        impl->cmds.resize(size);
        for (auto i = 0; i < size; ++i)
            impl->pools[i] = guard(device.createCommandPoolUnique({
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = family.idx,
            }));
    }

    Queue::~Queue() noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        guard(impl->queue.waitIdle());
    }

    auto Queue::allocate(cref<Pairs> pairs) noexcept -> obj<Buffer> {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        auto idx = stl::scheduler::index();
        auto& cmds = impl->cmds[idx];
        auto& pool = impl->pools[idx].get();

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

        auto begin = vk::CommandBufferBeginInfo{};
        if (picked) {
            guard(picked->impl->cmd->reset());
            guard(picked->impl->cmd->begin(&begin));
            picked->waits = pairs;
            picked->blocks.clear();
            picked->stages.clear();
            picked->waits.clear();
            picked->signals.clear();
            return picked;
        } else {
            auto cmd = make_obj<Buffer>();
            cmd->type = type;
            cmd->waits = pairs;
            cmd->blocks.cmd = cmd.get();
            cmd->impl->family = impl->families[u32(type)].idx;
            cmd->impl->cmd = std::move(guard(device.allocateCommandBuffersUnique({
                .commandPool = pool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1,
            })).front());
            guard(cmd->impl->cmd->begin(&begin));
            return cmd;
        }
    }

    auto Queue::submit(rref<obj<Buffer>> cmd, cref<Pairs> pairs) noexcept -> void {
        cmd->signals = pairs;
        guard(cmd->impl->cmd->end());
        auto collect = [](cref<Pairs> semaphores, vk::PipelineStageFlags2 flags) {
            auto collected = std::vector<vk::SemaphoreSubmitInfo>(semaphores.size());
            for (auto i = 0; i < collected.size(); ++i) {
                auto [timeline, count] = semaphores[i];
                collected[i] = vk::SemaphoreSubmitInfo{
                    .semaphore = timeline->impl->semaphore.get(),
                    .value = count,
                    .stageMask = flags,
                };
            };
            return collected;
        };

        auto waits = collect(cmd->waits, vk::PipelineStageFlagBits2::eTopOfPipe);
        auto signals = collect(cmd->signals, vk::PipelineStageFlagBits2::eBottomOfPipe);
        auto submit = vk::CommandBufferSubmitInfo{
            .commandBuffer = cmd->impl->cmd.get(),
        };

        auto info = vk::SubmitInfo2{
            .waitSemaphoreInfoCount = u32(waits.size()),
            .pWaitSemaphoreInfos = waits.data(),
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &submit,
            .signalSemaphoreInfoCount = u32(signals.size()),
            .pSignalSemaphoreInfos = signals.data(),
        };

        while (impl->flag.test_and_set(std::memory_order::acquire));
        guard(impl->queue.submit2(info));
        impl->flag.clear(std::memory_order::release);
        auto& cmds = impl->cmds[stl::scheduler::index()];
        cmds.push_back(std::move(cmd));
    }
}
