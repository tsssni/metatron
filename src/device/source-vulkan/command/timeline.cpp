#include "timeline.hpp"

namespace mtt::command {
    Timeline::Timeline() noexcept {
        auto& ctx = Context::instance();
        auto device = ctx.impl->device.get();
        auto info = vk::SemaphoreTypeCreateInfo{
            .semaphoreType = vk::SemaphoreType::eTimeline,
            .initialValue = 0,
        };
        impl->semaphore = guard(device.createSemaphoreUnique({.pNext = &info}));
    }

    auto Timeline::wait(u64 count, u64 timeout) noexcept -> bool {
        auto& ctx = Context::instance();
        auto device = ctx.impl->device.get();
        auto wait_info = vk::SemaphoreWaitInfo{
            .semaphoreCount = 1,
            .pSemaphores = &impl->semaphore.get(),
            .pValues = &count,
        };
        return device.waitSemaphores(&wait_info, timeout) == vk::Result::eSuccess;
    }

    auto Timeline::signal(u64 count) noexcept -> void {
        auto& ctx = Context::instance();
        auto device = ctx.impl->device.get();
        auto signal_info = vk::SemaphoreSignalInfo{
            .semaphore = impl->semaphore.get(),
            .value = count,
        };
        guard(device.signalSemaphore(&signal_info));
    }
}
