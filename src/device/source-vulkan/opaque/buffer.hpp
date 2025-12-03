#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/buffer.hpp>

namespace mtt::opaque {
    struct Buffer::Impl final {
        vk::UniqueBuffer buffer;
        vk::DeviceAddress address;
        Impl() noexcept = default;
        Impl(ref<stl::buf> buffer) noexcept;
        Impl(cref<stl::buf> buffer) noexcept;

    private:
        vk::UniqueBuffer staging;
        vk::UniqueDeviceMemory device_memory;
        vk::UniqueDeviceMemory host_memory;
    };
}
