#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/buffer.hpp>

namespace mtt::opaque {
    struct Buffer::Impl final {
        vk::MemoryAllocateFlags static flags;
        vk::BufferUsageFlags2 static usages;
        vk::UniqueBuffer device_buffer;
        vk::UniqueBuffer host_buffer;
        vk::UniqueDeviceMemory device_memory;
        vk::UniqueDeviceMemory host_memory;
    };
}
