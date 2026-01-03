#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/buffer.hpp>

namespace mtt::opaque {
    struct Buffer::Impl final {
        mtl<MTL::Buffer> device_buffer;
        mtl<MTL::Buffer> host_buffer;
    };
}
