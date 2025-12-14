#pragma once
#include "../opaque/buffer.hpp"
#include <metatron/device/encoder/transfer.hpp>

namespace mtt::encoder {
    struct Transfer_Encoder::Impl final {
        opaque::Barrier src_barrier;
        opaque::Barrier dst_barrier;
        mut<command::Buffer> cmd;

        auto stage(opaque::Buffer::View buffer) noexcept -> opaque::Buffer::View;
        auto flush(opaque::Buffer::View buffer) noexcept -> opaque::Buffer::View;
    };
}
