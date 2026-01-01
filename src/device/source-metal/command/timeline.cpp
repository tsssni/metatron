#include "timeline.hpp"

namespace mtt::command {
    Timeline::Timeline() noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        impl->event = device->newSharedEvent();
    }

    auto Timeline::wait(u64 count, u64 timeout) noexcept -> bool {
        return impl->event->waitUntilSignaledValue(count, timeout);
    }

    auto Timeline::signal(u64 count) noexcept -> void {
        impl->event->setSignaledValue(count);
    }
}
