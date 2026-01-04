#pragma once
#include "context.hpp"
#include <metatron/device/command/buffer.hpp>

namespace mtt::command {
    struct Buffer::Impl final {
        mut<MTL::CommandBuffer> cmd;
        mtl<MTL::Fence> fence;
    };
}
