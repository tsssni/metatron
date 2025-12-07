#include "recorder.hpp"
#include "queue.hpp"

namespace mtt::command {
    Retention::Retention() noexcept {}

    Retentions::~Retentions() noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        for (auto i = 0; i < Queue::num_types; ++i)
            guard(Queues::instance().queues[i].impl->queue.waitIdle());
    }

    auto Recorder::Impl::init() noexcept -> void {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        auto init_queue = [&ctx, device](Queue::Type type) {
            auto idx = u32(type);
            auto& queue = Queues::instance().queues[idx];
            auto& retention = Retentions::instance().retentions[idx];
            queue.family = type == Queue::Type::render
            ? ctx->render_queue : ctx->transfer_queue;
            queue.impl->queue = device.getQueue2({
                .queueFamilyIndex = queue.family,
                .queueIndex = 0,
            });
            retention.impl->pool = guard(device.createCommandPoolUnique({
                .queueFamilyIndex = queue.family,
            }));

            auto timeline_info = vk::StructureChain<
                vk::SemaphoreCreateInfo,
                vk::SemaphoreTypeCreateInfo
            >{};
            timeline_info.get<vk::SemaphoreTypeCreateInfo>() = {
                .semaphoreType = vk::SemaphoreType::eTimeline,
                .initialValue = 0,
            };
            queue.impl->timeline = guard(device.createSemaphoreUnique(timeline_info.get()));

            auto buffers = guard(device.allocateCommandBuffersUnique({
                .commandPool = retention.impl->pool.get(),
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = Retention::num_recorder,
            }));
            for (auto i = 0; i < Retention::num_recorder; ++i)
                retention.impl->buffers[i] = std::move(buffers[i]);
        };
        init_queue(Queue::Type::render);
        init_queue(Queue::Type::transfer);
    }

    Recorder::Recorder(Queue::Type type)noexcept: type(type) {
        auto& queue = Queues::instance().queues[u32(type)];
        auto& retention = Retentions::instance().retentions[u32(type)];
        timestamp = queue.timestamp.fetch_add(1);
        auto idx = timestamp % Retention::num_recorder;
        retention.update_buffers[idx].clear();
        retention.stage_buffers[idx].clear();

        impl->timestamp = timestamp;
        impl->queue = queue.impl->queue;
        impl->timeline = queue.impl->timeline.get();
        impl->pool = retention.impl->pool.get();
        impl->buffer = retention.impl->buffers[idx].get();

        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        auto static constexpr timeout = 1e9;
        guard(device.waitSemaphores({
            .semaphoreCount = 1,
            .pSemaphores = &impl->timeline,
            .pValues = &retention.last_timestamp[idx],
        }, timeout));
        auto begin = vk::CommandBufferBeginInfo{};
        guard(impl->buffer.begin(&begin));
    }

    Recorder::~Recorder() noexcept {
        auto idx = timestamp % Retention::num_recorder;
        auto& queue = Queues::instance().queues[u32(type)];
        auto& retention = Retentions::instance().retentions[u32(type)];
        retention.last_timestamp[idx] = timestamp;
        guard(impl->buffer.end());

        auto wait_infos = std::array<vk::SemaphoreSubmitInfo, Queue::num_types>{};
        for (auto i = 0; i < Queue::num_types; ++i)
            wait_infos[i] = {
                .semaphore = Queues::instance().queues[i].impl->timeline.get(),
                .value = wait_timestamps[i],
                .stageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
            };
        auto signal_info = vk::SemaphoreSubmitInfo{
            .semaphore = impl->timeline,
            .value = timestamp,
            .stageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
        };
        auto buffer_info = vk::CommandBufferSubmitInfo{
            .commandBuffer = impl->buffer,
        };
        auto info = vk::SubmitInfo2{
            .waitSemaphoreInfoCount = 2,
            .pWaitSemaphoreInfos = wait_infos.data(),
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &buffer_info,
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signal_info,
        };
        guard(impl->queue.submit2(info));
    }
}
