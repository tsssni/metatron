#pragma once
#include "../opaque/buffer.hpp"
#include <metatron/device/encoder/transfer.hpp>

namespace mtt::encoder {
    struct Transfer_Encoder::Impl final {
        opaque::Barrier src_barrier;
        opaque::Barrier dst_barrier;
        mut<command::Buffer> cmd;
    };
}
