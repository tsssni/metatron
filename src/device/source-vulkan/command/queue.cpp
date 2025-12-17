#include "queue.hpp"
#include "buffer.hpp"
#include "timeline.hpp"
#include <metatron/core/stl/thread.hpp>
#include <atomic>

namespace mtt::command {
    std::array<u32, num_types> Queue::Impl::family;
    std::array<u32, num_types> Queue::Impl::count;
    std::array<u32, num_types> Queue::Impl::idx;

    Queue::Queue(Type type) noexcept: type(type) {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        auto render = type == Type::render;
        auto family = impl->family[u32(type)];
        auto count = impl->count[u32(type)];

        auto idx = impl->idx[u32(type)];
        if (idx >= count) stl::abort("queue family {} overflow", u32(type));
        ++impl->idx[u32(type)];

        impl->queue = device.getQueue2({
            .queueFamilyIndex = family,
            .queueIndex = idx,
        });
        auto size = stl::scheduler::instance().size();
        impl->pools.resize(size);
        impl->cmds.resize(size);
        for (auto i = 0; i < size; ++i)
            impl->pools[i] = guard(device.createCommandPoolUnique({
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = family,
            }));
    }

    Queue::~Queue() noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        guard(impl->queue.waitIdle());
    }

    auto Queue::allocate() noexcept -> obj<Buffer> {
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
            picked->blocks.clear();
            picked->stages.clear();
            picked->waits.clear();
            picked->signals.clear();
            return picked;
        } else {
            auto cmd = make_obj<Buffer>();
            cmd->type = type;
            cmd->blocks.cmd = cmd.get();
            cmd->impl->family = impl->family[u32(type)];
            cmd->impl->cmd = std::move(guard(device.allocateCommandBuffersUnique({
                .commandPool = pool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1,
            })).front());
            guard(cmd->impl->cmd->begin(&begin));
            return cmd;
        }
    }

    auto Queue::submit(rref<obj<Buffer>> cmd) noexcept -> void {
        guard(cmd->impl->cmd->end());
        auto collect = [](cref<std::vector<Buffer::Timeline_Count>> semaphores) {
            auto collected = std::vector<vk::SemaphoreSubmitInfo>(semaphores.size());
            for (auto i = 0; i < collected.size(); ++i) {
                auto [timeline, count] = semaphores[i];
                collected[i] = vk::SemaphoreSubmitInfo{
                    .semaphore = timeline->impl->semaphore.get(),
                    .value = count,
                };
            };
            return collected;
        };
        auto waits = collect(cmd->waits);
        auto signals = collect(cmd->signals);
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
