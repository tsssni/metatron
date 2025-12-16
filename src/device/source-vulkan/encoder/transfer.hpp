#pragma once
#include "../opaque/buffer.hpp"
#include <metatron/device/encoder/transfer.hpp>

namespace mtt::encoder {
    struct Transfer_Encoder::Impl final {
        opaque::Barrier src_barrier;
        opaque::Barrier dst_barrier;
        mut<command::Buffer> cmd;

        template<typename T>
        auto transfer(u32 family, T view) noexcept -> void;
        template<typename T, typename U>
        auto copy(T to, U from) noexcept -> void;
        template<typename T>
        auto copy(T to, T from) noexcept -> void;
    };
}
