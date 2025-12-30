#pragma once
#include "context.hpp"
#include <metatron/device/command/allocator.hpp>

namespace mtt::command {
    struct Memory::Impl final {
        vk::UniqueDeviceMemory memory;
    };
}
