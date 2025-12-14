#pragma once
#include "context.hpp"
#include <metatron/device/command/buffer.hpp>

namespace mtt::command {
    struct Buffer::Impl final {
        u32 family;
        vk::UniqueCommandBuffer cmd;
    };
}
