#include "timeline.hpp"
#include <metatron/core/stl/print.hpp>

namespace mtt::command {
    Timeline::Timeline(bool shared) noexcept {
        auto& ctx = Context::instance().impl;
        auto device = ctx->device.get();
        if (shared) {
            impl->shared = device->newSharedEvent();
            impl->event = impl->shared->retain();
        } else impl->event = device->newEvent();
    }

    auto Timeline::wait(u64 count, u64 timeout) noexcept -> bool {
        return impl->shared->waitUntilSignaledValue(count, timeout);
    }

    auto Timeline::signal(u64 count) noexcept -> void {
        impl->shared->setSignaledValue(count);
    }
}
